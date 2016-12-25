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

#include "field.h"
#include "feedGenerator.h"

#define mp make_pair

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::pair;

typedef unsigned int64_t ui64;
typedef unsigned int ui32;

void calcLine(field& cur, field& next, int line) {
    cell curCell;
    ui32 px, py, numbFreeSpaceAround;
    ui64 numbFeedAround, numbStuff1Around, numbStuff2Around;
    bool freeSpaceAround[3][3];
    pair <ui64, pair <int, int> > feedAround[9];
    pair <ui64, pair <int, int> > stuff1Around[9];
    pair <ui64, pair <int, int> > stuff2Around[9];

    for (ui32 k = 0;k < cur.m;++k) {
        curCell = cur[line][k];
        if(curCell.exist1 || curCell.exist2) {
            numbFreeSpaceAround = 0;
            for(int x = -1; x <= 1; ++x) {
                for(int y = -1;y <= 1; ++y) {

                    px = (line + x + cur.n) % cur.n;
                    py = (k + y + cur.m) % cur.m;

                    freeSpaceAround[x][y] = 0;
                    if((x || y) && !curCell.exist2) {
                         freeSpaceAround[x][y] = 1;
                         ++numbFreeSpaceAround;
                    }

                    feedAround[3*x + y] = mp(cur.data[px][py].feed, mp(x,y));
                    stuff1Around[3*x + y] = mp(cur.data[px][py].stuff1, mp(x,y));
                    stuff2Around[3*x + y] = mp(cur.data[px][py].stuff2, mp(x,y));

                    numbFeedAround += feedAround[3*x + y].first;
                    numbStuff1Around += stuff1Around[3*x + y].first;
                    numbStuff2Around += stuff2Around[3*x + y].first;
                }
            }
            sort(numbFeedAround, numbFeedAround + 9);
            sort(numbStuff1Around, numbStuff1Around + 9);
            sort(numbStuff2Around, numbStuff2Around + 9);
            if(curCell.exist1) {
                ui64 needConsumeFeed = cur.vConsumeFeed;
                ui64 curConsumeFeed;
                for(int i =0;i < 9;++i) {
                    px = cur[numbFeedAround.second.first];
                    py = cur[numbFeedAround.second.second];
                    curConsumeFeed = min((needConsumeFeed+ (8 - i))/(9 - i), cur.data[px][py]].feed);
                    curConsumeFeed = min(curConsumeFeed, needConsumeFeed);
                    cur.data[px][py]].feed -= curConsumeFeed;
                    needConsumeFeed -= curConsumeFeed;
                }
                if (needConsumeFeed > 0) {
                    
                }
            }
        }
    }
}

field seriesCalc(field& f) {
    field workField[2];
    workField[0] = workField[1] = field;
    ui32 cur, n = f.n, m = f.m, stepLimit = f.stepLimit;

    for(ui32 t = 0; t < stepLimit; ++t) {
        cur = t & 1;
        for(ui32 i = 0; i < n; ++i) {
            calcLine(workField[cur], workField[1-cur], i);
        }
    }

    return workField[cur];
}
