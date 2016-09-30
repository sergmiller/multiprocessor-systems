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

#define MAX_NUMB_OF_THREADS 20
#define WAIT_TIME_MICROSECONDS 20

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

struct thread_info{
    int me_numb;
    int steps;
    int work_ind;
    int threads;
    field_simple_t* field;
};

typedef struct thread_info thread_info_t;

thread_info_t thread_info[MAX_NUMB_OF_THREADS];

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

void* parallel_calc_next(void* data) {
    field_simple_t* field = (*(thread_info_t*)data).field;
    int me_id = (*(thread_info_t*)data).me_numb;
    int steps = (*(thread_info_t*)data).steps;
    int numb_threads = (*(thread_info_t*)data).threads;
    int* work_ind = &(*(thread_info_t*)data).work_ind;
    int n = field->n;
    int m = field->m;

    for(int t = 0; t < steps; ++t) {

        char* prev_data = field->data[t & 1];
        char* cur_data = field->data[1 - t & 1];
        int cur_sum, nx, ny;


        for(int i = n/numb_threads * me_id; i < n/numb_threads * (me_id  + 1) + (i == numb_threads - 1); ++i) {
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
            ++done_work;
            *work_ind = 1;
            while(!*work_ind) {
                pthread_cond_wait(&cv, &mutex_step);
            }
        pthread_mutex_unlock(&mutex_step);
    }

    return NULL;
}


vector <vector <char> >  parallel_life_simple(char* field_data, int n, int m, int tm, int th) {
    field_simple_t field;
    init_simple_field(&field, field_data, n, m);

    pthread_mutex_init(&mutex_step, NULL);

    done_work = 0;

    for(int i = 0;i < th;++i) {
        thread_info[i].me_numb = i;
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
            if(done_work == th) {
                // if(do_draw) {
                //     visualize(&field);
                // }
                ++field.t;
                done_work = 0;
                    for(int i = 0; i < th;++i) {
                            thread_info[i].work_ind = 0;
                    }
                pthread_cond_broadcast(&cv);
            }
        pthread_mutex_unlock(&mutex_step);
        // printf("main_outside\n");
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
    return ans;
}
/**********************************SIMPLE********************************************/



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
        n = atoi(argv[0]);
        m = atoi(argv[1]);
        tm = atoi(argv[2]);
        th = atoi(argv[3]);
        p = atoi(argv[4]);
        q = atoi(argv[5]);
        if (n < m) {
            swap(n,m);
            swapcoord = true;
        }
        field = (char*)calloc(n * m, sizeof(char));
        init_random_field(&field,n,m,p,q);
    }

    vector <vector <char> > ans1;
    vector <vector <char> > ans2;
    cout << "n= " << n << " ,m= " << m << " ,time= " << tm << " ,threads= " << th << endl;
    
    clock_gettime(CLOCK_REALTIME, &begin);
    ans1 = parallel_life_simple(field, n, m, tm, th);
    clock_gettime(CLOCK_REALTIME, &end);

    int elapsed_seconds = end.tv_sec - begin.tv_sec;
    cout << "simple: " << elapsed_seconds << " s" << endl;

    clock_gettime(CLOCK_REALTIME, &begin);
    // ans2 = parallel_life_fine(field, n, m, tm, th);
    clock_gettime(CLOCK_REALTIME, &end);
    
    elapsed_seconds = end.tv_sec - begin.tv_sec;
    cout << "fine: " << elapsed_seconds << " s" << endl;
    cout << endl;

    // check(ans1,ans2);
    return 0;
}