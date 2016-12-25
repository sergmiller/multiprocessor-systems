#ifndef SERIES_CALC_H
#define SERIES_CALC_H

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
#include "feedGenerator.h"

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

void calcLine(field& f, ui32 line, ui32 t) {
    cell curCell;
    ui32 px, py, numbFreeSpaceAround, n = f.n, m = f.m;
    ui64 numbFeedAround, numbStuff1Around, numbStuff2Around;
    bool freeSpaceAround[3][3];
    pair <ui64, pair <int, int> > feedAround[9];
    pair <ui64, pair <int, int> > stuff1Around[9];
    pair <ui64, pair <int, int> > stuff2Around[9];

    for (ui32 k = 0;k < m;++k) {
        curCell = f.data[line][k];
        if(curCell.exist1 || curCell.exist2) {
            numbFreeSpaceAround = 0;
            for(int x = -1; x <= 1; ++x) {
                for(int y = -1;y <= 1; ++y) {

                    px = (line + x + n) % n;
                    py = (k + y + m) % m;

                    freeSpaceAround[x][y] = 0;
                    if((x || y) && !curCell.exist2) {
                         freeSpaceAround[x][y] = 1;
                         ++numbFreeSpaceAround;
                    }

                    feedAround[3*x + y] = mp(f.data[px][py].feed, mp(x,y));
                    stuff1Around[3*x + y] = mp(f.data[px][py].stuff1, mp(x,y));
                    stuff2Around[3*x + y] = mp(f.data[px][py].stuff2, mp(x,y));

                    numbFeedAround += feedAround[3*x + y].first;
                    numbStuff1Around += stuff1Around[3*x + y].first;
                    numbStuff2Around += stuff2Around[3*x + y].first;
                }
            }
            sort(feedAround, feedAround + 9);
            sort(stuff1Around, stuff1Around + 9);
            sort(stuff2Around, stuff2Around + 9);

            //consume part
            if(curCell.exist1) {
                ui64 needConsumeFeed = f.vConsumeFeed;
                ui64 curConsumeFeed;
                for(int i = 0;i < 9;++i) {
                    px = feedAround[i].second.first;
                    py = feedAround[i].second.second;
                    curConsumeFeed = min((needConsumeFeed + (8 - i))/(9 - i),
                        f.data[px][py].feed);
                    curConsumeFeed = min(
                        curConsumeFeed, needConsumeFeed);
                    f.data[px][py].feed -= curConsumeFeed;
                    needConsumeFeed -= curConsumeFeed;
                }
                if (needConsumeFeed > 0) {
                    ui64 needConsumeStuff2 = f.vConsumeStuff2 - f.vConsumeFeed
                        + needConsumeFeed;
                    ui64 curConsumeStuff2;
                    for(int i = 0;i < 9;++i) {
                        px = stuff2Around[i].second.first;
                        py = stuff2Around[i].second.second;
                        curConsumeStuff2 = min(
                            (needConsumeStuff2 + (8 - i))/(9 - i),
                            f.data[px][py].stuff2);
                        curConsumeStuff2 = min(
                            curConsumeStuff2, needConsumeStuff2);
                        f.data[px][py].stuff2 -= curConsumeStuff2;
                        needConsumeStuff2 -= curConsumeStuff2;
                    }
                    if(needConsumeStuff2 > 0) {
                        ++curCell.hungerRounds;
                        if(curCell.hungerRounds == curCell.maxHunger) {
                            curCell.exist1 = 0;
                        }
                    } else {
                        curCell.hungerRounds = 0;
                    }
                } else {
                    curCell.hungerRounds = 0;
                }
            }

            if(curCell.exist2) {
                ui64 needConsumeStuff1 = f.vConsumeFeed;
                ui64 curConsumeStuff1;
                for(int i = 0;i < 9;++i) {
                    px = stuff1Around[i].second.first;
                    py = stuff1Around[i].second.second;
                    curConsumeStuff1 = min(
                        (needConsumeStuff1 + (8 - i))/(9 - i),
                        f.data[px][py].stuff1);
                    curConsumeStuff1 = min(
                        curConsumeStuff1, needConsumeStuff1);
                    f.data[px][py].stuff1 -= curConsumeStuff1;
                    needConsumeStuff1 -= curConsumeStuff1;
                }
                if (needConsumeStuff1 > 0) {
                    curCell.exist2 = 0;
                }
            }

            //divide and pollution part
            if(curCell.stuff1 >= f.pollutionBound) {
                curCell.exist1 = 0;
            }

            if(curCell.stuff2 >= f.divisionBound && curCell.exist2) {
                int gen = rand() % numbFreeSpaceAround;
                for(int x = -1;x <= 1; ++x) {
                    for(int y = -1; y <= 1; ++y) {
                        if(!x && !y) {
                            continue;
                        }

                        px = (line + x + f.n) % f.n;
                        py = (k + y + f.m) % f.m;

                        if(!gen) {
                            f.data[px][py].exist2 = 1;
                            break;
                        }
                        --gen;
                    }
                    if(!gen)
                        break;
                }
            }

            //produce part
            if(curCell.exist2)
                curCell.stuff1 += f.vProduceStuff1;
            if(curCell.exist1)
                curCell.stuff2 += f.vProduceStuff2;
            vector <ui64> newStuff = feedGenerator(line,k,t,curCell);
            curCell.feed = newStuff[0];
            curCell.stuff1 = newStuff[1];
            curCell.stuff2 = newStuff[2];

            f.data[line][k] = curCell;
        }
    }
}

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

void visualize(field f) {
    system("clear");
    sleep(2);
    int n = f.n, m = f.m;
    for(ui32 i = 0;i < n;++i) {
        for(ui32 j = 0;j < m;++j) {
            ui32 sum = 2*f.data[i][j].exist1 + 2*f.data[i][j].exist2;
            if(sum == 3)
                printf("3");
            if(sum == 2)
                printf("2");
            if(sum == 1)
                printf("1");
            if(sum == 0)
                printf("-");
        }
        printf("\n");
    }
}

field seriesCalc(field f) {
    ui32 n = f.n, m = f.m, stepLimit = f.stepLimit;

    for(ui32 t = 0; t < stepLimit; ++t) {
        for(ui32 i = 0; i < n; ++i) {
            calcLine(f, i, t);
        }
    }

    visualize(f);

    return f;
}

#endif
