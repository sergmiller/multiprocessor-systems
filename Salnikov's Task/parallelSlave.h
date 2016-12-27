#ifndef PARALLEL_SLAVE_H
#define PARALLEL_SLAVE_H

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

// #include <omp.h>
// #include <mpi.h>

#include "field.h"
#include "tools.h"

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

void parallelSlave(ui32 th, ui32 rank,
        MPI_Status* status, MPI_Datatype* cell_type) {
    ui32 n,m;
    fieldData data;
    ui32 fieldSize[2];
    ui64 metaData[9];
    MPI_Bcast(&fieldSize, 2, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&metaData, 9, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    n = fieldSize[0], m = fieldSize[1];
    data.numbAlive1 = metaData[0];
    data.numbAlive2 = metaData[1];
    data.vConsumeFeed = metaData[2];
    data.vConsumeStuff1 = metaData[3];
    data.vConsumeStuff2 = metaData[4];
    data.vProduceStuff1 = metaData[5];
    data.vProduceStuff2 = metaData[6];
    data.divisionBound = metaData[7];
    data.pollutionBound = metaData[8];

    vector <vector <cell> > field_part;
    pair <int,int> bounds = get_bounds(rank, th, n);

    field_part.resize(bounds.second - bounds.first + 2, vector <cell>(m));

    for(int i = bounds.first; i < bounds.second; ++i) {
        MPI_Recv(&field_part[i-bounds.first + 1][0],
            m, cell_type, 0, 0, MPI_COMM_WORLD, status);
    }

    // MPI_Recv(&field_part[0][0], m, cell_type, 0, 1, MPI_COMM_WORLD, status);
    // MPI_Recv(&field_part[bounds.second - bounds.first + 1][0],
    //     m, cell_type, 0, 2, MPI_COMM_WORLD, status);

    int cur_part = 0;
    int prev, next, delta = bounds.second - bounds.first;
    prev = rank - 1;
    next = rank + 1;
    if(rank == 1) {
        prev = th - 1;
    }
    if(rank == th - 1) {
        next = 1;
    }

    MPI_Request export_request[4];
    MPI_Request import_request[4];

    for(int t = 0; t < tm; ++t) {
        for(int i = 2; i < delta - 2; ++i) {
            // calcLine(&field_part[cur_part], &field_part[1-cur_part],
            //   i, bounds.second - bounds.first + 2, m);
            parallelCalcLine(&field_part, data, i, t);
        }

        if(t) {
            MPI_Wait(&export_request[0], status);
            MPI_Wait(&import_request[0], status);
            MPI_Wait(&export_request[1], status);
            MPI_Wait(&import_request[1], status);
            MPI_Wait(&export_request[2], status);
            MPI_Wait(&import_request[2], status);
            MPI_Wait(&export_request[3], status);
            MPI_Wait(&import_request[3], status);
        }

        parallelCalcLine(&field_part, data, 1, t);
        parallelCalcLine(&field_part, data, delta-1, t);
        
        //Send first and second line to left neighboor
        MPI_Isend(&field_part[cur_part][0][0],
            m, cell_type, prev, 1, MPI_COMM_WORLD, &export_request[0]);
        MPI_Isend(&field_part[cur_part][1][0],
            m, cell_type, prev, 2, MPI_COMM_WORLD, &export_request[1]);

        //Revieve first and second line from right neighboor
        MPI_Irecv(&field_part[cur_part][delta][0], m, cell_type,
            next, 1, MPI_COMM_WORLD, &import_request[0]);
        MPI_Irecv(&field_part[cur_part][delta + 1][0], m, cell_type,
            next, 2, MPI_COMM_WORLD, &import_request[1]);

        parallelCalcLine(&field_part, data, delta-1, t);
        parallelCalcLine(&field_part, data, delta, t);

        //Send last and last - 1 line to right neighboor
        MPI_Isend(&field_part[cur_part][delta][0],
            m, cell_type, next, 3, MPI_COMM_WORLD, &export_request[2]);
        MPI_Isend(&field_part[cur_part][delta + 1][0],
                m, cell_type, next, 4, MPI_COMM_WORLD, &export_request[3]);

        //Revieve last and last - 1 line from left neighboor
        MPI_Irecv(&field_part[cur_part][0][0], m, cell_type,
            prev, 3, MPI_COMM_WORLD, &import_request[2]);
        MPI_Irecv(&field_part[cur_part][1][0], m, cell_type,
            prev, 4, MPI_COMM_WORLD, &import_request[3]);
    }

    MPI_Wait(&export_request[0], status);
    MPI_Wait(&export_request[1], status);
    MPI_Wait(&import_request[0], status);
    MPI_Wait(&import_request[1], status);
    MPI_Wait(&export_request[2], status);
    MPI_Wait(&export_request[3], status);
    MPI_Wait(&import_request[2], status);
    MPI_Wait(&import_request[3], status);

    for(int i = 0; i < delta; ++i) {
        MPI_Send(&field_part[i][0],
            m, cell_type, 0, 5, MPI_COMM_WORLD);
}

#endif
