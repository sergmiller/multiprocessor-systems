#ifndef PARALLEL_TOOLS_H
#define PARALLEL_TOOLS_H

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
#include <mpi.h>

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

pair<int,int> get_bounds(int r, int th, int n) {
    return std::make_pair(n/(th-1)*(r-1), n/(th-1)*r + (r==th-1) * (n%(th-1)));
}


void parallelCalcLine(vector <vector <cell> >& f, fieldData& data,
                                                        ui32 line, ui32 step) {
    cell curCell;
    ui32 px, py, numbFreeSpaceAround, m = f[0].size();
    ui64 numbFeedAround, numbStuff1Around, numbStuff2Around;
    int ind;
    pair <ui64, pair <int, int> > feedAround[9];
    pair <ui64, pair <int, int> > stuff1Around[9];
    pair <ui64, pair <int, int> > stuff2Around[9];

    for (ui32 k = 0;k < m;++k) {
        // cout << "k = " << k << endl;
        curCell = f[line][k];
        if(curCell.exist1 || curCell.exist2) {
            numbFreeSpaceAround = 0;
            for(int x = -1; x <= 1; ++x) {
                for(int y = -1;y <= 1; ++y) {
                    // cout << x << " " << y << endl;

                    px = line + x;
                    py = (k + y + m) % m;

                    if((x || y) && !f[px][py].exist2) {
                         ++numbFreeSpaceAround;
                    }

                    ind = 3*(x+1) + y + 1;
                    feedAround[ind] = mp(f[px][py].feed, mp(x,y));
                    stuff1Around[ind] = mp(f[px][py].stuff1, mp(x,y));
                    stuff2Around[ind] = mp(f[px][py].stuff2, mp(x,y));

                    numbFeedAround += feedAround[ind].first;
                    numbStuff1Around += stuff1Around[ind].first;
                    numbStuff2Around += stuff2Around[ind].first;
                }
            }
            sort(feedAround, feedAround + 9);
            sort(stuff1Around, stuff1Around + 9);
            sort(stuff2Around, stuff2Around + 9);
            //consume part
            if(curCell.exist1) {
                ui64 needConsumeFeed = data.vConsumeFeed;
                ui64 curConsumeFeed;
                for(int i = 0;i < 9;++i) {
                    px = line + feedAround[i].second.first;
                    py = (k + feedAround[i].second.second + m) % m;
                    curConsumeFeed = min((needConsumeFeed + (8 - i))/(9 - i),
                        f[px][py].feed);
                    curConsumeFeed = min(
                        curConsumeFeed, needConsumeFeed);
                    f[px][py].feed -= curConsumeFeed;
                    needConsumeFeed -= curConsumeFeed;
                }
                if (needConsumeFeed > 0) {
                    ui64 needConsumeStuff2 = data.vConsumeStuff2
                        - data.vConsumeFeed
                        + needConsumeFeed;
                    ui64 curConsumeStuff2;
                    for(int i = 0;i < 9;++i) {
                        px = line + stuff2Around[i].second.first;
                        py = (k + stuff2Around[i].second.second + m) % m;
                        curConsumeStuff2 = min(
                            (needConsumeStuff2 + (8 - i))/(9 - i),
                            f[px][py].stuff2);
                        curConsumeStuff2 = min(
                            curConsumeStuff2, needConsumeStuff2);
                        f[px][py].stuff2 -= curConsumeStuff2;
                        needConsumeStuff2 -= curConsumeStuff2;
                    }
                    if(needConsumeStuff2 > 0) {
                        cout << "hunger " << curCell.hungerRounds
                                    << " / " << curCell.maxHunger << endl;
                        if(curCell.hungerRounds == curCell.maxHunger) {
                            curCell.exist1 = 0;
                            curCell.hungerRounds = 0;
                            --data.numbAlive1;
                        }
                        ++curCell.hungerRounds;
                    } else {
                        curCell.hungerRounds = 0;
                    }
                } else {
                    curCell.hungerRounds = 0;
                }
            }

            if(curCell.exist2) {
                ui64 needConsumeStuff1 = data.vConsumeFeed;
                ui64 curConsumeStuff1;
                for(int i = 0;i < 9;++i) {
                    px = line + stuff1Around[i].second.first;
                    py = (k + stuff1Around[i].second.second + m) % m;
                    curConsumeStuff1 = min(
                        (needConsumeStuff1 + (8 - i))/(9 - i),
                        f[px][py].stuff1);
                    curConsumeStuff1 = min(
                        curConsumeStuff1, needConsumeStuff1);
                    f[px][py].stuff1 -= curConsumeStuff1;
                    needConsumeStuff1 -= curConsumeStuff1;
                }
                if (needConsumeStuff1 > 0) {
                    curCell.exist2 = 0;
                    --data.numbAlive2;
                }
            }
            //divide and pollution part
            if(curCell.stuff1 >= data.pollutionBound && curCell.exist1) {
                curCell.exist1 = 0;
                --data.numbAlive1;
            }

            if(curCell.stuff2 >= data.divisionBound
                && curCell.exist2 && numbFreeSpaceAround) {
                int gen = rand() % numbFreeSpaceAround;
                bool add = false;
                for(int x = -1;x <= 1; ++x) {
                    for(int y = -1; y <= 1; ++y) {
                        if(!x && !y) {
                            continue;
                        }

                        px = line + x;
                        py = (k + y + m) % m;

                        if(f[px][py].exist2)
                            continue;

                        if(!gen) {
                            f[px][py].exist2 = 1;
                            ++data.numbAlive2;
                            add = true;
                            break;
                        }
                        --gen;
                    }
                    if(add)
                        break;
                }
            }
            //produce part
            if(curCell.exist2)
                curCell.stuff1 += data.vProduceStuff1;
            if(curCell.exist1)
                curCell.stuff2 += data.vProduceStuff2;
            vector <ui64> newStuff = feedGenerator(line,k,step,curCell);
            curCell.feed = newStuff[0];
            curCell.stuff1 = newStuff[1];
            curCell.stuff2 = newStuff[2];

            f[line][k] = curCell;
        }
    }
}

#endif
