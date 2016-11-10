//
//  lifeomp.cpp
//  OpenMP Life
//
//  Created by Serg Miller on 10.11.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cassert>

#include <omp.h>
#include "series.h"
#include "tools.h"

#define INPUT "input.txt"

vector <vector<char> > life_omp(vector <vector<char> >& field, int n, int m, int tm, int th) {
    vector <vector <char> > omp_ans[2];
    omp_ans[0] = field;
    omp_ans[1] = field;
    int cur_part = 0;
    
    for(int t = 0;t < tm;++t) {
#pragma omp parallel for num_threads(th)
        for(int i = 0;i < n;++i) {
            calc_next(&omp_ans[cur_part], &omp_ans[1-cur_part], i, n, m);
        }
#pragma end parallel
        cur_part = 1 - cur_part;
    }
    return omp_ans[cur_part];
}

int main(int argc, const char * argv[]) {
    // insert code here...
    srand((unsigned int)time(NULL));
    int n,m,tm,th;
    double p;// p  is probability of alive
    bool swapcoord = 0;
    
    if (argc == 1) {
        freopen(INPUT, "r", stdin);
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
        p = atof(argv[5]);
        if (n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    }
    
    double start, dur;
    vector<vector<char> > mpi_ans;
    vector<vector<char> > series_ans;
    vector <vector <char> > field;
    
    
    field.resize(n,vector <char> (m));
    init_random_field(field,n,m,p);
    
    start = omp_get_wtime();
    series_ans = life_s_simple(field, n, m, tm);
    dur = omp_get_wtime() - start;
    cout << "series time: " << (int)dur << "s" << endl;

    
  
    start = omp_get_wtime();
    mpi_ans = life_omp(field, n, m, tm, th);
    dur = omp_get_wtime() - start;
    cout << "omp time: " << (int)dur << "s" << endl;
    test(series_ans,mpi_ans);
    cout << "n = " << n << ", m = " << m << ", tm = " << tm << ", th = " << th << endl;
    
    return 0;
}
