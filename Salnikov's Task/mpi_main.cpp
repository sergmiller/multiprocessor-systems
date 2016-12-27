#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include <math.h>

// #include <omp.h>
// #include <mpi.h>

#include "field.h"
#include "parallelCalc.h"
#include "tools.h"

#define STATE "state_fix.txt"
#define CREATURES "creatures_fix.conf"
#define FINAL "final_state.txt"
#define STEPLIMIT_MAIN 1000

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::string;


//in argv fixed input files in strong format without comments
int main(int argc, char const *argv[]) {
    srand((unsigned int)time(NULL));
    std::ios::sync_with_stdio(false);
    string startState = STATE, creatures = CREATURES, finalSate = FINAL;
    ui32 steps = STEPLIMIT_MAIN;
    //read custom I/O files and step limit
    if(argc > 1) {
        steps = atoi(argv[1]);
        if(argc > 3) {
            startState = string(argv[2]);
            creatures = string(argv[3]);
            if(argc > 4) {
                finalSate = string(argv[4]);
            }
        }
    }

    ui32 th, rank;

    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&th);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    //definition cell for mpi
    const int nitems = 7;
    int blocklen[7] = {1,1,1,1,1,1,1};
    MPI_Datatype types[7] = {
        MPI_UNSIGNED_LONG,
        MPI_UNSIGNED_LONG,
        MPI_UNSIGNED_LONG,
        MPI_UNSIGNED,
        MPI_UNSIGNED,
        MPI_UNSIGNED,
        MPI_UNSIGNED
    };
    MPI_Datatype mpi_cell_type;
    MPI_Aint offsets[7];

    offsets[0] = offsetof(cell, feed);
    offsets[1] = offsetof(cell, stuff1);
    offsets[2] = offsetof(cell, stuff2);
    offsets[3] = offsetof(cell, hungerRounds);
    offsets[4] = offsetof(cell, maxHunger);
    offsets[5] = offsetof(cell, exist1);
    offsets[6] = offsetof(cell, exist2);

    MPI_Type_create_struct(nitems, blocklen, offsets, types, &mpi_cell_type);
    MPI_Type_commit(&mpi_cell_type);

    if(!rank) {
    //init field with data from I files
    field f(startState, creatures, steps);
    field res = parallelCalc(f, &mpi_cell_type);
    writeStateToFile(finalSate, res);
    } else
        parallelSlave(th, rank, &status, &mpi_cell_type);

    MPI_Type_free(&mpi_cell_type);
    MPI_Finalize();
    return 0;
}
