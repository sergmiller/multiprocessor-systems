//
//  parallel_simple.h
//  Game Life
//
//  Created by Serg Miller on 08.10.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#ifndef parallel_simple_h
#define parallel_simple_h

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <pthread.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>

#include "series.h"

//#define MAX_NUMB_OF_THREADS 20
#define CYCLE_LEN 20

using std::vector;
using std::cin;
using std::cout;
using std::endl;

struct field_simple {
    vector <vector <char> > data[2];
    int done_all_works;
    field_simple(vector <vector <char> >& _data,  int th) {
        data[0] = _data;
        data[1] = _data;
        done_all_works = 0;
    }
};

struct thread_info_simple {
    pthread_mutex_t* mutex_p;
    pthread_cond_t* cv_p;
    pthread_cond_t* main_cv_p;
    int steps;
    int me_id;
    int th;
    int done_work;
    field_simple& field;
    thread_info_simple(field_simple& _field, pthread_mutex_t* _mutex_p,    pthread_cond_t* _cv_p, pthread_cond_t* _main_cv_p, int _id, int _steps, int _th): field(_field) {
        me_id = _id;
        mutex_p = _mutex_p;
        cv_p = _cv_p;
        main_cv_p = _main_cv_p;
        steps = _steps;
        th = _th;
        done_work = 0;
    }
};

pthread_cond_t main_cv;


void* calc_next_p_simple(void* void_info_simple_p) {
    thread_info_simple* info_t = (thread_info_simple*)void_info_simple_p;
    field_simple& field = info_t->field;
    
    int me_id = info_t->me_id;
    int steps = info_t->steps;
    int th = info_t->th;
    int n = (int)field.data[0].size();
    int m = (int)field.data[0][0].size();
    int* done_work_p = &info_t->done_work;
    int* done_all_works_p = &field.done_all_works;
    
    pthread_mutex_t* mutex_p = info_t->mutex_p;
    pthread_cond_t* cv_p = info_t->cv_p;
    pthread_cond_t* main_cv_p = info_t->main_cv_p;
    
    int cur, next;
    int lb = n/th * me_id;
    int rb = n/th * (me_id + 1) + (me_id + 1 == th) * (n % th);
    for(int t = 0;t < steps;++t) {
        cur = t & 1;
        next = 1 - cur;
        
        for(int i = lb; i < rb; ++i) {
            calc_next(field.data[cur], field.data[next], i, n, m);
        }
        
        pthread_mutex_lock(mutex_p);
        
        *done_work_p = 1;
        ++*done_all_works_p;
        if(*done_all_works_p == th) {
            pthread_cond_broadcast(main_cv_p);
        }
        
        while(*done_work_p) {
            pthread_cond_wait(cv_p, mutex_p);
        }
        
        pthread_mutex_unlock(mutex_p);
    }
    
    return (void*)NULL;
}

vector <vector <char> >  life_p_simple(vector <vector <char> > & data, int n, int m, int tm, int th) {
    vector <pthread_t> threads(th);
    vector <thread_info_simple> info;
    pthread_mutex_t mutex;
    pthread_cond_t cv, main_cv;
    
    field_simple field(data, th);
    info.reserve(th);
    
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cv, NULL);
    pthread_cond_init(&main_cv, NULL);
    for(int i = 0; i < th; ++i) {
        info.push_back(thread_info_simple(field, &mutex, &cv, &main_cv, i, tm, th));
        if(pthread_create(&threads[i], NULL, calc_next_p_simple, &info[i])) {
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
            ++t;
            field.done_all_works = 0;
            for(int i = 0; i < th;++i) {
                info[i].done_work = 0;
            }
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
    
    
//    vector <vector <char> > ans;
//    ans.resize(n, vector <char> (m));
//    int cur = tm & 1;
//    for(int i = 0;i < n;++i) {
//        for(int j = 0;j < m;++j) {
//            ans[i][j] = field.data[cur][i][j];
//        }
//    }
    cout << "out" << endl;
    return field.data[tm & 1];
}

#endif /* parallel_simple_h */
