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
#include "seriesCalc.h"

#define STATE "state_fix.txt"
#define CREATURES "creatures_fix.conf"
#define FINAL "final_state.txt"
#define STEPLIMIT 1000

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
    ui32 steps = STEPLIMIT;
    //read custom I/O files
    if(argc > 1) {
        assert(argc > 2);
        startState = string(argv[1]);
        creatures = string(argv[2]);
        if(argc > 3) {
            finalSate = string(argv[3]);
            if(argc > 4) {
                steps = atoi(argv[4]);
            }
        }
    }

    //init field with data from I files
    field f(startState, creatures, steps);

    field res = seriesCalc(f);
    return 0;
}
