#ifndef FIELD_H
#define FIELD_H

#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <stdio.h>

#include <math.h>

// #include <omp.h>
// #include <mpi.h>

using std::cin;
using std::cout;
using std::endl;
using std::swap;
using std::vector;
using std::string;

typedef unsigned long long ui64;
typedef unsigned int ui32;

#define STEPLIMIT 100

//full information about of cell on field
struct cell {
    ui64 feed;
    ui64 stuff1;
    ui64 stuff2;
    ui32 hungerRounds;
    ui32 maxHunger;
    ui32 exist1;
    ui32 exist2;
    cell() {
        feed = stuff1 = stuff2 = hungerRounds = 0;
        maxHunger = exist1 = exist2 = 0;
    }
};

struct fieldData {
    ui64 numbAlive1, numbAlive2;
    ui64 vConsumeFeed, vConsumeStuff1, vConsumeStuff2;
    ui64 vProduceStuff1, vProduceStuff2;
    ui64 divisionBound, pollutionBound;
};

struct field {
    ui32 n, m, step, stepLimit;
    ui64 numbAlive1, numbAlive2;
    ui64 vConsumeFeed, vConsumeStuff1, vConsumeStuff2;
    ui64 vProduceStuff1, vProduceStuff2;
    ui64 divisionBound, pollutionBound;
    vector <vector <cell> > data;
    bool turned;
    field(string stateFile, string creaturesFile, ui32 _stepLimit = STEPLIMIT) {
        step = 0;
        turned = 0;
        numbAlive1 = numbAlive2 = 0;
        stepLimit = _stepLimit;
        FILE* fp;
        fp = freopen(stateFile.data(), "r", stdin);
        ui64 maxT;
        cin >> n >> m >> maxT;
        if(n < m) {
            turned = 1;
            swap(n,m);
        }
        data.resize(n, vector <cell> (m));
        ui32 x,y;
        cell curCell;
        for(ui64 t = 0;t < maxT;++t) {
            cin >> x >> y;
            if(turned)
                swap(x,y);
            cin >> curCell.feed >> curCell.stuff1 >> curCell.stuff2
            >> curCell.exist1 >> curCell.maxHunger >> curCell.exist2;
            data[x][y] = curCell;
            numbAlive1 += curCell.exist1;
            numbAlive2 += curCell.exist2;
            cout << numbAlive1 << " " << curCell.exist1 << endl;
        }
        fclose(fp);
        fp = freopen(creaturesFile.data(), "r", stdin);
        cin >> pollutionBound >> divisionBound >> vConsumeFeed
        >> vConsumeStuff1 >> vConsumeStuff2 >> vProduceStuff1 >> vProduceStuff2;
        assert(vConsumeStuff2 > vConsumeFeed);
        fclose(fp);
    }
};

#endif
