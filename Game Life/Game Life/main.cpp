//
//  main.cpp
//  Game Life
//
//  Created by Serg Miller on 08.10.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <chrono>

#include "series.h"
#include "parallel_simple.h"
#include "parallel_fine.h"

using std::vector;
using std::cin;
using std::cout;
using std::endl;

#define INPUT "input.txt"


void init_random_field(vector <vector <char> >& field,  int n, int m, float p) {
//    cout << p << endl;
//    cout <<RAND_MAX * p << endl;
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            field[i][j] = (rand() <= ((float)RAND_MAX * p));
//            cout << (int)field[i][j] << " ";
        }
//        cout << endl;
    }
}

void test(vector <vector <char> >& a, vector <vector <char> >& b) {
    int n = (int)a.size();
    int m = (int)a[0].size();
    assert(a.size() == b.size());
    assert(a[0].size() == b[0].size());
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            assert(a[i][j] == b[i][j]);
        }
    }
    cout << "OK" << endl;
}


int main(int argc, const char * argv[]) {
    srand((unsigned int)time(NULL));
    int n,m,tm,th,cycle;
    double p;// p  is probability of alive
    vector <vector <char> > field;
    bool swapcoord = 0;
    if (argc == 1) {
        freopen(INPUT, "w", stdout);
        cin >> n >> m >> tm >> th >> p>> cycle;
        if(n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    } else {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        tm = atoi(argv[3]);
        th = atoi(argv[4]);
        p = atof(argv[5]);
        cycle = atoi(argv[6]);
        if (n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    }

    field.resize(n,vector <char> (m));
    init_random_field(field,n,m,p);
    
    vector<vector <char> > ans_s_simple, ans_p_simple, ans_p_fine;
    
    cout << "n = " << n << " ,m = " << m << " ,time = " << tm << " ,threads = " << th << " , cycle= " << cycle << endl;
    
//
//    struct timespec begin, end;
//    long elapsed_seconds;
//    
//    clock_gettime(CLOCK_REALTIME, &begin);
//    ans_s_simple = life_s_simple(field, n, m, tm);
//    clock_gettime(CLOCK_REALTIME, &end);
//    
//    elapsed_seconds = end.tv_sec - begin.tv_sec;
//    cout << "serial simple: " << elapsed_seconds << " s" << endl;
//    
//    
//    clock_gettime(CLOCK_REALTIME, &begin);
//    ans_p_simple = life_p_simple(field, n, m, tm, th);
//    clock_gettime(CLOCK_REALTIME, &end);
//    
//    elapsed_seconds= end.tv_sec - begin.tv_sec;
//    cout << "parallel simple: " << elapsed_seconds << " s" << endl;
//    
//    
//    clock_gettime(CLOCK_REALTIME, &begin);
//    ans_p_fine = life_p_fine(field, n, m, tm, th, cycle);
//    clock_gettime(CLOCK_REALTIME, &end);
//    
//    elapsed_seconds = end.tv_sec - begin.tv_sec;
//    cout << "parallel fine: " << elapsed_seconds << " s" << endl;
    
    
    std::chrono::time_point<std::chrono::system_clock> start, end;
    int elapsed_seconds;

    start = std::chrono::system_clock::now();
    ans_s_simple = life_s_simple(field, n, m, tm);
    end = std::chrono::system_clock::now();
    
    elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>
    (end-start).count();
    cout << "serial simple: " << elapsed_seconds << " ms" << endl;
    
    
    start = std::chrono::system_clock::now();
    ans_p_simple = life_p_simple(field, n, m, tm, th);
    end = std::chrono::system_clock::now();

    elapsed_seconds= std::chrono::duration_cast<std::chrono::milliseconds>
    (end-start).count();
    cout << "parallel simple: " << elapsed_seconds << " ms" << endl;
    
    
    start = std::chrono::system_clock::now();
    ans_p_fine = life_p_fine(field, n, m, tm, th, cycle);
    end = std::chrono::system_clock::now();
    
    elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>
    (end-start).count();
    cout << "parallel fine: " << elapsed_seconds << " ms" << endl;
    
    test(ans_s_simple, ans_p_simple);
    
    test(ans_s_simple, ans_p_fine);
    
    return 0;
}
