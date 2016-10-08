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

#include "series.h"
#include "parallel_simple.h"
#include "parallel_fine.h"

using std::vector;
using std::cin;
using std::cout;
using std::endl;

#define INPUT "input.txt"

void init_random_field(vector <vector <char> >& field,  int n, int m, float p) {
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            field[i][j] = (rand() <= RAND_MAX * p);
        }
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
}


int main(int argc, const char * argv[]) {
    srand((unsigned int)time(NULL));
    int n,m,tm,th;
    double p;// p  is probability of alive
    vector <vector <char> > field;
    bool swapcoord = 0;
    if (argc == 1) {
        freopen(INPUT, "w", stdout);
        cin >> n >> m >> tm >> th >> p;
        if(n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    } else {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        tm = atoi(argv[3]);
        th = atoi(argv[4]);
        p = atoi(argv[5]);
        if (n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    }

    field.resize(n,vector <char> (m));
    init_random_field(field,n,m,p);
    
    struct timespec begin, end;
    
    
    
    vector<vector <char> > ans_s_simple, ans_p_simple, ans_p_fine;
    
    cout << "n = " << n << " ,m = " << m << " ,time = " << tm << " ,threads = " << th << endl;
    
    long elapsed_seconds;
    
    clock_gettime(CLOCK_REALTIME, &begin);
    ans_s_simple = life_s_simple(field, n, m, tm);
    clock_gettime(CLOCK_REALTIME, &end);
    
    elapsed_seconds = end.tv_sec - begin.tv_sec;
    cout << "serial simple: " << elapsed_seconds << " s" << endl;
    
    
    clock_gettime(CLOCK_REALTIME, &begin);
    ans_p_simple = life_p_simple(field, n, m, tm, th);
    clock_gettime(CLOCK_REALTIME, &end);

    elapsed_seconds= end.tv_sec - begin.tv_sec;
    cout << "parallel simple: " << elapsed_seconds << " s" << endl;
    
    
    clock_gettime(CLOCK_REALTIME, &begin);
    ans_p_fine = life_p_fine(field, n, m, tm, th);
    clock_gettime(CLOCK_REALTIME, &end);
    
    elapsed_seconds = end.tv_sec - begin.tv_sec;
    cout << "parallel fine: " << elapsed_seconds << " s" << endl;
    
    test(ans_s_simple, ans_p_simple);
    
    test(ans_s_simple, ans_p_fine);
    
    return 0;
}
