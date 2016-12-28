#ifndef PARALLEL_CALC_H
#define PARALLEL_CALC_H

#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <math.h>

#include "omp.h"
#include <mpi.h>

#include "field.h"
#include "tools.h"
#include "parallelTools.h"

#define mp make_pair

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::pair;
using std::sort;
using std::min;
using std::make_pair;

typedef unsigned long long ui64;
typedef unsigned int ui32;


// ui64 modelConsumeStuff(cell& curCell, field& cur, ui64 needConsumeStuff,
//   pair <ui64, pair <int, int> >* stuffAround) {
//     ui64 needConsumeStuff = curCell.vConsumeFeed;
//     ui64 curConsumeStuff;
//     for(int i = 0;i < 9;++i) {
//         px = stuffAround.second.first;
//         py = stuffAround.second.second;
//         curConsumeStuff = min(
//          (needConsumeStuff + (8 - i))/(9 - i), stuff[px][py]);
//         curConsumeStuff = min(curConsumeStuff, needConsumeStuff);
//         cur.data[px][py]].stuff1 -= curConsumeStuff1;
//         needConsumeStuff -= curConsumeStuff;
//     }
// }


field parallelCalc(field f, ui32 th,
                    MPI_Datatype cell_type, MPI_Status* status) {
    ui32 n = f.n, m = f.m, stepLimit = f.stepLimit;
    ui32 fieldSize[2] = {n,m};
    ui64 metaData[9];

    metaData[0] = f.numbAlive1;
    metaData[1] = f.numbAlive2;
    metaData[2] = f.vConsumeFeed;
    metaData[3] = f.vConsumeStuff1;
    metaData[4] = f.vConsumeStuff2;
    metaData[5] = f.vProduceStuff1;
    metaData[6] = f.vProduceStuff2;
    metaData[7] = f.divisionBound;
    metaData[8] = f.pollutionBound;

    MPI_Bcast(&fieldSize, 2, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&metaData, 9, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    // for(ui32 t = 0; t < stepLimit; ++t) {
    //     visualize(f);
    //
    //     cout << t + 1 << " " << f.numbAlive1 << " " <<  f.numbAlive2 << endl;
    //     for(ui32 i = 0; i < n; ++i) {
    //         parallelCalcLine(f, i);
    //     }
    //
    //     if(f.numbAlive1 + f.numbAlive2 == 0) {
    //         visualize(f);
    //         cout << "final step: " << f.step + 1 << endl;
    //         break;
    //     }
    //
    //     ++f.step;
    // }


//    for(int i = 0;i < n;++i) {
//        for(int j = 0;j < m;++j) {
//            cout << (int)field[i][j] << " ";
//        }
//        cout << endl;
//    }
    pair<int,int> bounds;

    for(int i = 1; i < th;++i) {
        bounds = get_bounds(i,th,n);
    //    cout << bounds.first << " " << bounds.second << endl;
        for(int j = bounds.first; j < bounds.second; ++j) {
            MPI_Send(&f.data[j][0], m, cell_type, i, 0, MPI_COMM_WORLD);
        }
    }

    vector <ui64> numbAlive1(th-1);
    vector <ui64> numbAlive2(th-1);
    ui64 summ;
    for(int i = 0;i < f.stepLimit; ++i) {
        summ = 0;
        MPI_Gather(NULL, 1, MPI_UNSIGNED_LONG, numbAlive1,
            th - 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
        #pragma omp parallel for numb_threads(10)
        for(int j = 0;j < th -1;++j) {
            summ += numbAlive1[j];
        }
        cout << i << " " << summ << endl;
    }

    for(int i = 1;i < th;++i) {
        bounds = get_bounds(i,th,n);
        // cout << "master " << i << endl;
        for(int j = bounds.first; j < bounds.second; ++j) {
            MPI_Recv(&f.data[j][0], m, cell_type, i,
                5, MPI_COMM_WORLD, status);
        }
    }

    return f;
}


#endif
