#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include <math.h>

#include <omp.h>
#include <mpi.h>

using std::cout;
using std::endl;
using std::vector;
using std::string;

typedef unsigned int64_t ui64;
typedef unsigned int ui32;


//full information about of cell on field
struct cell {
    ui64 feedConcentration;
    ui64 stuff1;
    ui64 stuff2;
    ui32 hungerRounds;
    ui32 maxHunger;
    char exist1;
    char exist2;
    cell() {
        feedConcentration = stuff1 = stuff2 = hungerRounds = 0;
        maxHunger = exist1 = exist2 = 0;
    }
};

struct field {
    ui32 n, m, stepLimit, numbAlive;
    ui32 vConsumeFeed, vConsumeStuff1, vConsumeStuff2;
    ui32 vProduceStuff1, vProduceStuff2;
    ui32 divisionBound, pollutionBound;
    vector <vector <cell> > data;
    bool turned;
    field(string stateFile, string cretureFile, ui32 _stepLimit = STEPLIMIT) {
        turned = 0;
        numbAlive = 0;
        stepLimit = _stepLimit;
        FILE* fp;
        fp = freopen(stateFile, "r", stdin);
        ui64 maxT;
        cin >> n >> m >> maxT;
        if(n < m) {
            turned = 1;
            swap(n,m);
        }

        data.resize(n, vector <cell> m);
        ui64 x,y;
        cell curCell;
        for(ui64 t = 0;t < maxT;++t) {
            cin >> x >> y;
            if(turned)
                swap(x,y);
            cin >> curCell.feedConcentration >> curCell.stuff1 >> curCell.stuff2
            >> curCell.exist1 >> curCell.maxHunger >> curCell.exist2;
            data[x][y] = curCell;
            numbAlive += (curCell.exist1 + curCell.exist2);
        }
        fclose(fp);
        fp = freopen(creturesFile, "r", stdin);
        cin >> pollutionBound >> divisionBound >> vConsumeFeed
        >> vConsumeStuff1 >> vConsumeStuff2 >> vProduceStuff1 >> vProduceStuff2;
        assert(vConsumeStuff2 > vConsumeFeed);
        fclose(fp);
    }
};
