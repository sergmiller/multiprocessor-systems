//
//  parallel_fine.h
//  Game Life
//
//  Created by Serg Miller on 08.10.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#ifndef parallel_fine_h
#define parallel_fine_h

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>

#include "series.h"

//#define MAX_NUMB_OF_THREADS 20
//#define CYCLE_LEN 20

using std::vector;
using std::cin;
using std::cout;
using std::endl;

struct field_fine {
    vector < vector <vector <char> > > data;
    vector <vector <int> > partly_calced;
    int done_all_works;
    int step;
    int cycle;
    field_fine(vector <vector <char> >& _data,  int th, int _cycle) {
        cycle = _cycle;
        partly_calced.resize(cycle);
        data.resize(cycle);
        for(int i = 0;i < cycle; ++i) {
            data[i] = _data;
            partly_calced[i].resize(th,-1);
        }
        done_all_works = 0;
        step = 0;
    }
};

struct thread_info_fine {
    pthread_mutex_t* mutex_p;
    pthread_cond_t* cv_p;
    pthread_cond_t* main_cv_p;
    int steps;
    int me_id;
    int th;
    int done_work;
    field_fine* field_t;
    thread_info_fine(field_fine* _field_t, pthread_mutex_t* _mutex_p,    pthread_cond_t* _cv_p, pthread_cond_t* _main_cv_p, int _id, int _steps, int _th) {
        me_id = _id;
        mutex_p = _mutex_p;
        cv_p = _cv_p;
        main_cv_p = _main_cv_p;
        steps = _steps;
        th = _th;
        done_work = 0;
        field_t = _field_t;
    }

};


void* calc_next_p_fine(void* void_info_fine_p) {
    thread_info_fine* info_t = (thread_info_fine*)void_info_fine_p;
    field_fine* field_t = info_t->field_t;
    
    int cycle = field_t->cycle;
    int me_id = info_t->me_id;
    int steps = info_t->steps;
    int th = info_t->th;
    int n = (int)field_t->data[0].size();
    int m = (int)field_t->data[0][0].size();
    int* done_work_p = &info_t->done_work;
    int* done_all_works_p = &field_t->done_all_works;
    int* step_p = &field_t->step;
    
    pthread_mutex_t* mutex_p = info_t->mutex_p;
    pthread_cond_t* cv_p = info_t->cv_p;
    pthread_cond_t* main_cv_p = info_t->main_cv_p;
    
    int t = 0;
    int offset;
    int cur_calced;
    int back_flag;
    int cur, next;
    int lb = n/th * me_id;
    int rb = n/th * (me_id + 1) + (me_id + 1 == th) * (n % th);
    int lb0,rb0,lb1,rb1;
    int end_flag;
    
    while(t <= steps) {
        back_flag = 0;
        end_flag = 0;
        
        pthread_mutex_lock(mutex_p);
//        assert(t >= *step_p);
        if(!*done_work_p && t > *step_p) {
            back_flag = 1;
        } else {
            if(*step_p == steps) {
                end_flag = 1;
            } else {
                if(t == steps || t + 1 - *step_p == cycle) {
                    while(*done_work_p) {
                        pthread_cond_wait(cv_p, mutex_p);
                    }
                    back_flag = 1;
                }
            }
        }
        
        cur_calced = field_t->partly_calced[(t + 1) % cycle][me_id];
        offset = t - *step_p;
        pthread_mutex_unlock(mutex_p);

        if(end_flag) {
            break;
        }
        
        if(back_flag) {
            t = *step_p;
            continue;
        }
        
//        pthread_mutex_lock(mutex_p);
//        cout << "id " << me_id << " t " << t << " offset " << offset << " cur_calced " << cur_calced << " cur done work " << *done_all_works_p << endl;
//        pthread_mutex_unlock(mutex_p);

        lb0 = lb + offset;
        rb0 = lb + cur_calced;
        lb1 = rb - cur_calced;
        rb1 = rb - offset;
        
        if(cur_calced == -1) {
            rb0 = lb0;
            lb1 = rb0;
        }
        
        cur = t % cycle;
        next = (t + 1) % cycle;
    
        for(int i = lb0; i < rb0; ++i) {
            calc_next(&field_t->data[cur], &field_t->data[next], i, n, m);
        }
        
        for(int i = lb1; i < rb1; ++i) {
            calc_next(&field_t->data[cur], &field_t->data[next], i, n, m);
        }
        
        field_t->partly_calced[(t+1)%cycle][me_id] = offset;
        
        pthread_mutex_lock(mutex_p);
        if(!offset) {
            *done_work_p = 1;
            ++*done_all_works_p;
            if(*done_all_works_p == th) {
                pthread_cond_broadcast(main_cv_p);
//                while(*step_p == t) {
//                    pthread_cond_wait(cv_p, mutex_p); // last thread waits while main thread updates data for next step
//                }
            }
        }
        pthread_mutex_unlock(mutex_p);
        
        ++t;
    }
    
    return (void*)NULL;
}

vector <vector <char> >  life_p_fine(vector <vector <char> > & data, int n, int m, int tm, int th, int cycle) {
    vector <pthread_t> threads(th);
    vector <thread_info_fine> info;
    pthread_mutex_t mutex;
    pthread_cond_t cv, main_cv;
    
    field_fine field(data, th, cycle);
    info.reserve(th);
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cv, NULL);
    pthread_cond_init(&main_cv, NULL);
    
    for(int i = 0; i < th; ++i) {
        info.push_back(thread_info_fine(&field, &mutex, &cv, &main_cv, i, tm, th));
        if(pthread_create(&threads[i], NULL, calc_next_p_fine, &info[i])) {
            cout << "ERROR: CAN'T CREATE THREAD" << endl;
            return vector <vector <char> > ();
        }
    }

    int t = 0;
    while(t < tm) {
        pthread_mutex_lock(&mutex);
        
        while(field.done_all_works != th) {
            pthread_cond_wait(&main_cv, &mutex);
        }
        ++field.step;
        field.done_all_works = 0;
//        cout << "next turn: " << t + 1 << endl;
        for(int i = 0; i < th;++i) {
            info[i].done_work = 0;
            field.partly_calced[t % cycle][i] = -1;
        }
        ++t;
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&mutex);
    }
    
    int dum;
    
    for(int i = 0;i < th;++i) {
        if(pthread_join(threads[i],(void**)&dum)) {
            cout << "ERROR: CAN'T JOIN THREAD" << endl;
            return vector <vector <char> > ();
        }
    }
    
//    cout << "out" << endl;
//    field.data[tm & 1][0][0] += 10;
    return field.data[tm % cycle];
}

#endif /* parallel_fine_h */
