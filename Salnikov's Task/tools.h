#ifndef TOOLS_H
#define TOOLS_H

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

void writeStateToFile(string filename, field& f) {
    FILE* fp = freopen(filename.data(), "w", stdout);
    cout << "last step: " << f.step + 1 << endl;
    for(ui32 i = 0;i < f.n; ++i) {
        for(ui32 j = 0;j < f.m; ++j) {
            cout << i << " " << j << " " << f.data[i][j].exist1 << " "
            << f.data[i][j].exist2 << " " << f.data[i][j].hungerRounds << " "
            << f.data[i][j].feed << " " << f.data[i][j].stuff1 <<  " "
            << f.data[i][j].stuff2 << endl;
        }
    }
    fclose(fp);
}

#endif
