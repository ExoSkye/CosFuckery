#include <pthread.h>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <csignal>

pthread_mutex_t mtx;
uint8_t done;

float fcos(float x) {
    float a;

    asm(
            "fcos;" : "=&t" (a) : "f" (x)
    );

    return a;
}

struct startData {
    uint32_t start;
    uint8_t step;
};

void* thread(void* data) {
    float buffer[16384][3];
    int k = 0;

    startData start_data = *(startData*)data;

    for (uint32_t i = start_data.start; i < UINT32_MAX; i += start_data.step) {
        float x = *(float*)&i;

        float asm_val = fcos(x);
        float real = cos(x);

        if (!(real - 0.0001 < asm_val && real + 0.0001 > asm_val)) {
            if (k >= 16384) {
                char num[128] = { 0 };
                sprintf(num, "%u.dat", i);
                FILE* f = fopen(num, "w");

                for (auto & j : buffer) {
                    fwrite(&j[0], sizeof(float), 1, f);
                    fwrite("\xFF", sizeof(char), 1, f);
                    fwrite(&j[1], sizeof(float), 1, f);
                    fwrite("\xFF", sizeof(char), 1, f);
                    fwrite(&j[2], sizeof(float), 1, f);
                    fwrite("\xFF", sizeof(char), 1, f);
                    fwrite("\xFF", sizeof(char), 1, f);
                }

                fflush(f);
                fclose(f);

                k = 0;
            }

            buffer[k][0] = x;
            buffer[k][1] = real;
            buffer[k][2] = asm_val;
            k++;
        }
    }

    pthread_mutex_lock(&mtx);
    done += 1;
    pthread_mutex_unlock(&mtx);

    free(data);

    pthread_exit(NULL);
}

int main() {
    pthread_mutex_init(&mtx, nullptr);

    uint8_t concurrency = sysconf(_SC_NPROCESSORS_ONLN);

    pthread_t* threads = (pthread_t*)malloc(concurrency * sizeof(pthread_t));

    for (uint32_t i = 0; i < concurrency; i++) {
        startData* data = (startData*)malloc(sizeof(startData));

        data->start = i;
        data->step = concurrency;

        pthread_create(&threads[i], nullptr, &thread, (void*)data);
    }

    while (done != concurrency) {
        usleep(100000);
    }

    pthread_mutex_destroy(&mtx);
    free(threads);

    return 0;
}
