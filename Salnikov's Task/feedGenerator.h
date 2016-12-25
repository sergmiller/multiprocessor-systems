#ifndef FEED_GENERATOR_H
#define FEED_GENERATOR_H

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

using std::cout;
using std::endl;
using std::vector;
using std::string;

vector <ui64> feedGenerator(ui32 x, ui32 y, ui32 t, cell cur) {
    vector <ui64> res(3);
    res[0] = cur.feed;
    res[1] = cur.stuff1;
    res[2] = cur.stuff2;
    return res;
}

#endif
