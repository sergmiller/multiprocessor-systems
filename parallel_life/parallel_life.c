#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include <memory.h>

#define TM 3000
#define NUMB_OF_THREADS 4
#define WAIT_TIME_MICROSECONDS 20

struct field{
    int step;
    int size;
    int qnt;
    char* data[2];
};

typedef struct field field_t;


struct thread_info{
    int me_numb;
    int steps;
    int work_ind;
    field_t* field;
};

typedef struct thread_info thread_info_t;

pthread_t threads[NUMB_OF_THREADS];
pthread_mutex_t mutex_step;
pthread_cond_t cv;
int done_work;
thread_info_t thread_info[NUMB_OF_THREADS];
int dirs[8][2] = {{0,1}, {1,0}, {-1,0}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

void init_field_with_data(field_t* field, int _size, int _qnt, int* x, int* y) {
    field->step = 0;
    field->size = _size;
    field->qnt = _qnt;
    field->data[0] = (char*)calloc(_size * _size, sizeof(char));
    field->data[1] = (char*)malloc(_size * _size * sizeof(char));
    for(int i = 0;i < _qnt;++i) {
        *(field->data[0] + x[i] * _size + y[i]) = 1;
    }
}

void init_field_with_rand(field_t* field, int _size, int p) {
    srand(time(NULL));
    field->step = 0;
    field->size = _size;
    field->qnt = 0;
    field->data[0] = (char*)calloc(_size * _size, sizeof(char));
    field->data[1] = (char*)malloc(_size * _size * sizeof(char));
    int cur;
    for(int i = 0;i < _size; ++i) {
        for(int j = 0;j < _size;++j) {
            cur = rand()%p;
            if(cur) {
             *(field->data[0] + i*_size + j) = 1;
             ++field->qnt;
            }
        }
    }
}

void calc_next(field_t* field) {
    int size = field->size;
    int cur_sum, nx, ny;

    ++field->step;

    char* prev_data = field->data[1 - field->step & 1];
    char* cur_data = field->data[field->step & 1];

    for(int i = 0; i < size; ++i) {
        for(int j = 0; j < size; ++j) {
            cur_sum = 0;

            for(int k = 0; k < 8; ++k) {
                nx = (i + dirs[k][0] + size) % size;
                ny = (j + dirs[k][1] + size) % size;
                if(*(prev_data + size * nx + ny)) {
                    ++cur_sum;
                }
            }

            char prev_point = *(prev_data + size * i + j);

            if((!prev_point && cur_sum == 3) || (prev_point && (cur_sum == 2 || cur_sum == 3))) {
                 *(cur_data + size * i + j) = 1;
            } else {
                 *(cur_data + size * i + j) = 0;
            }
        }
    }
}

void visualize(field_t* field) {
    system("clear");
    int size = field->size;
    char* cur_data = field->data[field->step & 1];
    for(int i = 0;i < size;++i) {
        for(int j = 0;j < size;++j) {
            if(*(cur_data + size * i + j)) {
                printf("# ");
            } else {
                printf("- ");
            }
        }
        printf("\n");
    }
}

void* parallel_calc_next(void* data) {
    field_t* field = (*(thread_info_t*)data).field;
    int size = field->size;
    int me_id = (*(thread_info_t*)data).me_numb;
    int steps = (*(thread_info_t*)data).steps;
    int* work_ind = &(*(thread_info_t*)data).work_ind;

    for(int t = 0; t < steps; ++t) {

        char* prev_data = field->data[t & 1];
        char* cur_data = field->data[1 - t & 1];
        int cur_sum, nx, ny;


        for(int i = size/NUMB_OF_THREADS * me_id; i < size/NUMB_OF_THREADS * (me_id  + 1) + (i == NUMB_OF_THREADS - 1); ++i) {
            for(int j = 0;j < size; ++j) {
                 cur_sum = 0;

                for(int k = 0; k < 8; ++k) {
                    nx = (i + dirs[k][0] + size) % size;
                    ny = (j + dirs[k][1] + size) % size;
                    if(*(prev_data + size * nx + ny)) {
                        ++cur_sum;
                    }
                }

                char prev_point = *(prev_data + size * i + j);

                if((!prev_point && cur_sum == 3) || (prev_point && (cur_sum == 2 || cur_sum == 3))) {
                     *(cur_data + size * i + j) = 1;
                } else {
                     *(cur_data + size * i + j) = 0;
                }
            }
        }

        pthread_mutex_lock(&mutex_step);
            ++done_work;
            *work_ind = 1;
            while(!*work_ind) {
                pthread_cond_wait(&cv, &mutex_step);
            }
        pthread_mutex_unlock(&mutex_step);
    }

    return NULL;
}

void parallel_life(int size, int steps, int do_draw, int start_alive, int prob, int* x, int* y) {
   
    field_t field;
    if(start_alive == -1) {
        init_field_with_rand(&field, size, prob);
    } else { 
        init_field_with_data(&field, size, start_alive, x, y);
    }

    if(do_draw) {
        visualize(&field);
        usleep(TM);
    }

    double start_time = clock();

    pthread_mutex_init(&mutex_step, NULL);

    done_work = 0;

    for(int i = 0;i < NUMB_OF_THREADS;++i) {
        thread_info[i].me_numb = i;
        thread_info[i].steps = steps;
        thread_info[i].field = &field;
        thread_info[i].work_ind = 0;
        if(pthread_create(threads + i, NULL, parallel_calc_next, thread_info + i)) {
            printf("ERROR_CREATE_THREAD\n");
            return;
        }
    }
    
    while(field.step < steps) {
        // calc_next(&field);
        pthread_mutex_lock(&mutex_step);
            // printf("main_inside\n");
            if(done_work == NUMB_OF_THREADS) {
                if(do_draw) {
                    visualize(&field);
                }
                ++field.step;
                done_work = 0;
                    for(int i = 0; i < NUMB_OF_THREADS;++i) {
                        // pthread_spin_lock(&mutex_work_ind[i]);
                            (thread_info + i)->work_ind = 0;
                        // pthread_mutex_unlock(&mutex_work_ind[i]);
                    }
                pthread_cond_broadcast(&cv);
            }
        pthread_mutex_unlock(&mutex_step);
        // printf("main_outside\n");
        // usleep(rand()%(WAIT_TIME_MICROSECONDS));
        usleep(WAIT_TIME_MICROSECONDS);
    }

    int status;

    for(int i = 0;i < NUMB_OF_THREADS; ++i) {
        if(pthread_join(threads[i],(void**)&status)) {
            printf("ERROR_JOIN_THREAD\n");
            return;
        }
    }

    double times_per_sec = (clock() - start_time)/CLOCKS_PER_SEC;
    printf("PARALLEL_MODE: threads %d, steps: %d, size: %d, work time: %f\n", NUMB_OF_THREADS, steps, size, times_per_sec);
}

void life(int size, int steps, int do_draw, int start_alive, int prob, int* x, int* y) {
   
    field_t field;
    if(start_alive == -1) {
        init_field_with_rand(&field, size, prob);
    } else { 
        init_field_with_data(&field, size, start_alive, x, y);
    }

    if(do_draw) {
        visualize(&field);
        usleep(TM);
    }

    double start_time = clock();
    
    while(field.step < steps) {
        calc_next(&field);
        if(do_draw) {
            visualize(&field);
            usleep(TM);
        }
        ++field.step;
    }

    double times_per_sec = (clock() - start_time)/CLOCKS_PER_SEC;
    printf("ONETHREAD_MODE: threads %d, steps: %d, size: %d, work time: %f\n", 1, steps, size, times_per_sec);
}

void solve(int argc, char** argv) {
    assert(argc >= 5 || argc == 1);
    int size, steps, do_draw, start_alive, prob = 2;
    int* x = NULL;
    int* y = NULL;
    if(argc == 1) {
        freopen("input.txt", "r", stdin);
        scanf("%d%d%d%d",&size, &steps, &do_draw, &start_alive);
    } else {
        size = atoi(argv[1]);
        steps = atoi(argv[2]);
        do_draw = atoi(argv[3]);
        start_alive = atoi(argv[4]);
    }
   
    field_t field;
    if(start_alive == -1) {
        if(argc >= 6) {
            prob = atoi(argv[5]);
        }
        // init_field_with_rand(&field, size, p);
    } else {
        assert(argc > 4 + 2*start_alive || argc == 1);        
        x = (int*)malloc(start_alive * sizeof(int));
        y = (int*)malloc(start_alive * sizeof(int));
        for(int i = 0;i < start_alive; ++i) {
            if(argc == 1) {
                scanf("%d%d", &x[i], &y[i]);
            } else {
                x[i] = atoi(argv[2*i + 5]);
                y[i] = atoi(argv[2*i + 6]);
            }
        }
        // init_field_with_data(&field, size, start_alive, x, y);
    }

    parallel_life(size, steps, do_draw, start_alive, prob, x, y);
    life(size, steps, do_draw, start_alive, prob, x, y);
    printf("\n");
}

int main(int argc, char** argv) {
    srand(time(NULL));
    // freopen("output.txt", "w", stdout);
    // char[5] = {"0","1000","0","-1","5"};
    double t = clock();
    solve(argc, argv);
    // char c = '#';
    // printf("%c\n",c);
    // printf("%c\n",c-1);
    printf("t: %f\n", (clock() - t)/CLOCKS_PER_SEC);
    return 0;
}
