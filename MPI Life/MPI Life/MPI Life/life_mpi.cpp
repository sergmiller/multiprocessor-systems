//
//  life_mpi.cpp
//  MPI Life
//
//  Created by Serg Miller on 29.10.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cassert>

#include <mpi.h>
#include "tools.h"
#include "series.h"
#include "master.h"
#include "slave.h"
#include "quick_slave.h"

using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::pair;

#define INPUT "input.txt"

int main(int argc, char * argv[]) {
    srand((unsigned int)time(NULL));
    int n,m,tm,th,fl,seed;
    double p;// p  is probability of alive
    int rank;
    bool swapcoord = 0;

    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&th);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (argc == 1) {
        freopen(INPUT, "r", stdin);
        cin >> n >> m >> tm >> p >> fl >> seed;
        if(n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    } else {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        tm = atoi(argv[3]);
        p = atof(argv[4]);
        fl = atoi(argv[5]);
        seed = atoi(argv[6]);
        if (n < m) {
            std::swap(n,m);
            swapcoord = true;
        }
    }

    vector<vector<char> > mpi_ans;
    vector<vector<char> > mpi_quick_ans;
    vector<vector<char> > series_ans;
    vector <vector <char> > field;

    if(!rank) {
        field.resize(n,vector <char> (m));
        init_random_field(field,n,m,p);
        if(fl) {
          double start = MPI_Wtime();
          series_ans = life_s_simple(field, n, m, tm);
          double dur = MPI_Wtime() - start;
          cout << dur << ", ";
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(!rank) {
        double start = MPI_Wtime();
        mpi_ans = MPI_Master(field, n, m, tm, th, rank, &status);
        double dur = MPI_Wtime() - start;
        cout << dur << "," << endl;
        if(fl) {
          test(series_ans,mpi_ans);
        }
    }
    else
        MPI_Slave(n, m, tm, th, rank, &status);

    MPI_Barrier(MPI_COMM_WORLD);

    if(!rank) {
        double start = MPI_Wtime();
        mpi_quick_ans = MPI_Master(field, n, m, tm, th, rank, &status);
        double dur = MPI_Wtime() - start;
        cout << dur << "," << endl;
        // if(fl) {
          test(mpi_ans,mpi_quick_ans);
        // }
    }
    else
        MPI_Quick_slave(n, m, tm, th, rank, &status);

    MPI_Finalize();
    return 0;
}
