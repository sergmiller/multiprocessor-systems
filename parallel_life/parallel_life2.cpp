#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#define DEPTH_SIZE 10
#define MAX_NUMB_OF_THREADS 20
#define WAIT_TIME_MICROSECONDS 10

using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::swap;

struct timespec begin, end;

/*******************COMMON THINGS*************************/

void init_random_field(char** field, int n, int m, int p, int q) {
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            *(*field + i * m + j) = (rand() % q >= p);
        }
    }
}

void check(vector <vector <char> >& ans1, vector <vector <char> >& ans2) {
    assert(ans1.size() == ans2.size() && ans1[0].size() == ans2[0].size());
    int n = ans1.size(), m = ans1[0].size();
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            assert(ans1[i][j] == ans2[i][j]);
        }
    }
}

/*******************COMMON THINGS*************************/


/**********************************SIMPLE********************************************/
struct field_simple{
    int t,n,m;
    char* data[2];
};

typedef struct field_simple field_simple_t;

pthread_mutex_t mutex_step;
pthread_cond_t cv;
int done_work;
pthread_t threads[MAX_NUMB_OF_THREADS];
int dirs[8][2] = {{0,1}, {1,0}, {-1,0}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

struct thread_info_simple{
    int me_numb;
    int steps;
    int work_ind;
    int threads;
    field_simple_t* field;
};

typedef struct thread_info_simple thread_info_simple_t;

thread_info_simple_t thread_info_s[MAX_NUMB_OF_THREADS];

void init_simple_field(field_simple_t* field, char* data, int n, int m) {
    field->t = 0;
    field->n = n;
    field->m = m;
    field->data[0] = (char*)calloc(n * m, sizeof(char));
    field->data[1] = (char*)malloc(n * m * sizeof(char));
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            *(field->data[0] + m * i + j) = *(data + m * i + j);
        }
    }
}

void* parallel_calc_next(void* void_data_p) {
    thread_info_simple_t* data_p = (thread_info_simple_t*)void_data_p;
    field_simple_t* field = data_p->field;
    int me_id = data_p->me_numb;
    int steps = data_p->steps;
    int numb_threads = data_p->threads;
    int* work_ind = &data_p->work_ind;
    int n = field->n;
    int m = field->m;

    for(int t = 0; t < steps; ++t) {

        char* prev_data = field->data[t & 1];
        char* cur_data = field->data[1 - t & 1];
        int cur_sum, nx, ny;

        for(int i = n/numb_threads * me_id; i < n/numb_threads * (me_id  + 1) + (i == numb_threads - 1) * (n % numb_threads); ++i) {
            for(int j = 0;j < m; ++j) {
                 cur_sum = 0;

                for(int k = 0; k < 8; ++k) {
                    nx = (i + dirs[k][0] + n) % n;
                    ny = (j + dirs[k][1] + m) % m;
                    if(*(prev_data + m * nx + ny)) {
                        ++cur_sum;
                    }
                }

                char prev_point = *(prev_data + m * i + j);

                if((!prev_point && cur_sum == 3) || (prev_point && (cur_sum == 2 || cur_sum == 3))) {
                     *(cur_data + m * i + j) = 1;
                } else {
                     *(cur_data + m * i + j) = 0;
                }
            }
        }

        pthread_mutex_lock(&mutex_step);
            // cout << me_id << " inside " << t << " step" << endl;
            ++done_work;
            *work_ind = 1;
            // cout << "work_ind " << *work_ind << endl;
            while(*work_ind) {
                // cout << me_id << " wait " << t << " step" << endl;
                pthread_cond_wait(&cv, &mutex_step);
                // cout << me_id << " up " << t << " step" << endl;
            }
            // cout << me_id << " after " << t << " step" << endl;
        pthread_mutex_unlock(&mutex_step);
    }

    return NULL;
}

vector <vector <char> >  parallel_life_simple(char* field_data, int n, int m, int tm, int th) {
    field_simple_t field;
    init_simple_field(&field, field_data, n, m);

    pthread_mutex_init(&mutex_step, NULL);
    pthread_cond_init(&cv, NULL);

    done_work = 0;

    for(int i = 0;i < th;++i) {
        thread_info_s[i].me_numb = i;
        thread_info[i].steps = tm;
        thread_info[i].field = &field;
        thread_info[i].work_ind = 0;
        thread_info[i].threads = th;
        if(pthread_create(threads + i, NULL, parallel_calc_next, thread_info + i)) {
            cout << "ERROR_CREATE_THREAD" << endl;
            return vector <vector <char> > ();
        }
    }
    
    while(field.t < tm) {
        pthread_mutex_lock(&mutex_step);
        // cout << done_work << " " << field.t <<  endl;
            if(done_work == th) {
                // cout << "done_work: " << done_work << " th: " << th << endl;
                // if(do_draw) {
                //     visualize(&field);
                // }
                // cout << "turn: " << field.t << endl;
                ++field.t;
                done_work = 0;
                for(int i = 0; i < th;++i) {
                        (thread_info + i)->work_ind = 0;
                }
                pthread_cond_broadcast(&cv);
            }
        pthread_mutex_unlock(&mutex_step);
        // cout << "main_outside" << endl;
        // usleep(rand()%(WAIT_TIME_MICROSECONDS));
        usleep(WAIT_TIME_MICROSECONDS);
    }

    vector <vector <char> > ans;
    // ans.resize(n, vector <char> m);
    // int cur = tm % 2;
    // for(int i = 0;i < n;++i) {
    //     for(int j = 0;j < m;++j) {
    //         ans[i][j] = field.data[i * m + j];
    //     }
    // }
    // cout << "out" << endl;
    return ans;
}
/**********************************SIMPLE********************************************/


/**********************************FINE**********************************************/

struct field_fine{
    int t,n,m;
    char* data[DEPTH_SIZE];
    char partialy_calc[MAX_NUMB_OF_THREADS][DEPTH_SIZE];
};

typedef struct field_fine field_fine_t;

struct thread_info_fine{
    int me_numb;
    int steps;
    int work_ind;
    int threads;
    field_fine_t* field;
};

typedef struct thread_info_fine thread_info_fine_t;

thread_info_fine_t thread_info_f;

int cur_turn[MAX_NUMB_OF_THREADS];

void init_fine_field(field_simple_t* field, char* data, int n, int m) {
    field->t = 0;
    field->n = n;
    field->m = m;
    field->data[0] = (char*)calloc(n * m, sizeof(char));
    for(int i = 1;i < DEPTH_SIZE;++i) {
        field->data[i] = (char*)malloc(n * m * sizeof(char));
        for(int j = 0;j < MAX_NUMB_OF_THREADS;++j) {
            field->partialy_calc[j][i] = 0;
        }
    }
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            *(field->data[0] + m * i + j) = *(data + m * i + j);
        }
    }
}


void* parallel_calc_next_fine(void* void_data_p) {
    thread_info_fine_t* data_p = (thread_info_fine_t*)void_data_p;
    field_fine_t* field = data_p->field;
    int me_id = data_p->me_numb;
    int steps = data_p->steps;
    int numb_threads = data_p->threads;
    int* work_ind = &data_p->work_ind;
    int n = field->n;
    int m = field->m;

    for(int t = 0; t < steps; ++t) {

        char* prev_data = field->data[t%DEPTH_SIZE];
        field->par
        char* cur_data = field->data[(t+1)%DEPTH_SIZE];
        int cur_sum, nx, ny;
        int offset = 0;

        for(int i = n/numb_threads * me_id + offset; i < n/numb_threads * (me_id  + 1) + (i == numb_threads - 1) * (n % numb_threads) - offset; ++i) {
            for(int j = 0;j < m; ++j) {
                 cur_sum = 0;

                for(int k = 0; k < 8; ++k) {
                    nx = (i + dirs[k][0] + n) % n;
                    ny = (j + dirs[k][1] + m) % m;
                    if(*(prev_data + m * nx + ny)) {
                        ++cur_sum;
                    }
                }

                char prev_point = *(prev_data + m * i + j);

                if((!prev_point && cur_sum == 3) || (prev_point && (cur_sum == 2 || cur_sum == 3))) {
                     *(cur_data + m * i + j) = 1;
                } else {
                     *(cur_data + m * i + j) = 0;
                }
            }
        }

        pthread_mutex_lock(&mutex_step);
            // cout << me_id << " inside " << t << " step" << endl;
            ++done_work;
            *work_ind = 1;
            // cout << "work_ind " << *work_ind << endl;
            while(*work_ind) {
                // cout << me_id << " wait " << t << " step" << endl;
                pthread_cond_wait(&cv, &mutex_step);
                // cout << me_id << " up " << t << " step" << endl;
            }
            // cout << me_id << " after " << t << " step" << endl;
        pthread_mutex_unlock(&mutex_step);
    }

    return NULL;
}

vector <vector <char> >  parallel_life_fine(char* field_data, int n, int m, int tm, int th) {
    field_fine_t field;
    init_fine_field(&field, field_data, n, m);

    for(int i = 0;i < th;++i) {
        cur_turn[i] = 0;
    }

    pthread_mutex_init(&mutex_step, NULL);
    pthread_cond_init(&cv, NULL);

    done_work = 0;

    for(int i = 0;i < th;++i) {
        thread_info[i].me_numb = i;
        thread_info[i].steps = tm;
        thread_info[i].field = &field;
        thread_info[i].work_ind = 0;
        thread_info[i].threads = th;
        if(pthread_create(threads + i, NULL, parallel_calc_next_fine, thread_info + i)) {
            cout << "ERROR_CREATE_THREAD" << endl;
            return vector <vector <char> > ();
        }
    }
    
    while(field.t < tm) {
        pthread_mutex_trylock(&mutex_step);
        // cout << done_work << " " << field.t <<  endl;
            if(done_work == th) {
                // cout << "done_work: " << done_work << " th: " << th << endl;
                // if(do_draw) {
                //     visualize(&field);
                // }
                // cout << "turn: " << field.t << endl;
                ++field.t;
                done_work = 0;
                for(int i = 0; i < th;++i) {
                        (thread_info + i)->work_ind = 0;
                }
                pthread_cond_broadcast(&cv);
            }
        pthread_mutex_unlock(&mutex_step);
        // cout << "main_outside" << endl;
        // usleep(rand()%(WAIT_TIME_MICROSECONDS));
        // usleep(WAIT_TIME_MICROSECONDS);
    }


    vector <vector <char> > ans;
    // ans.resize(n, vector <char> m);
    // int cur = tm % 2;
    // for(int i = 0;i < n;++i) {
    //     for(int j = 0;j < m;++j) {
    //         ans[i][j] = field.data[i * m + j];
    //     }
    // }
    // cout << "out" << endl;
    return ans;
}

/**********************************FINE**********************************************/


int main(int argc, char** argv) {
    srand(time(NULL));
    int n,m,tm,th, p,q; // p/q  is probability of alive
    bool swapcoord = 0;
    char* field;
    if (argc == 1) {
        freopen("input.txt", "w", stdout);
        cin >> n >> m >> tm >> th;
        if(n < m) {
            swap(n,m);
            swapcoord = true;
        }

        // field.resize(n,vector <char> m);
        field = (char*)calloc(n * m, sizeof(char));

        for(int i = 0;i < n;++i) {
            for(int j = 0;j < m;++j) {
                if(!swapcoord)
                    cin >> field[i*m + j];
                else
                    cin >> field[i*m + j];
            }
        }
    } else {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        tm = atoi(argv[3]);
        th = atoi(argv[4]);
        p = atoi(argv[5]);
        q = atoi(argv[6]);
        if (n < m) {
            swap(n,m);
            swapcoord = true;
        }
        field = (char*)calloc(n * m, sizeof(char));
    }

    init_random_field(&field,n,m,p,q);
    vector <vector <char> > ans1;
    vector <vector <char> > ans2;
    cout << "n = " << n << " ,m = " << m << " ,time = " << tm << " ,threads = " << th << endl;
    
    // clock_gettime(CLOCK_REALTIME, &begin);
    ans1 = parallel_life_simple(field, n, m, tm, th);
    // clock_gettime(CLOCK_REALTIME, &end);

    int elapsed_seconds = end.tv_sec - begin.tv_sec;
    cout << "simple: " << elapsed_seconds << " s" << endl;

    // clock_gettime(CLOCK_REALTIME, &begin);
    // ans2 = parallel_life_fine(field, n, m, tm, th);
    // clock_gettime(CLOCK_REALTIME, &end);
    
    elapsed_seconds = end.tv_sec - begin.tv_sec;
    cout << "fine: " << elapsed_seconds << " s" << endl;
    cout << endl;

    // check(ans1,ans2);
    return 0;
}
