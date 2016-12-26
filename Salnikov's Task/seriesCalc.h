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
    int ind;
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

                    if((x || y) && !f.data[x][y].exist2) {
                         ++numbFreeSpaceAround;
                    }

                    ind = 3*(x+1) + y + 1;
                    feedAround[ind] = mp(f.data[px][py].feed, mp(x,y));
                    stuff1Around[ind] = mp(f.data[px][py].stuff1, mp(x,y));
                    stuff2Around[ind] = mp(f.data[px][py].stuff2, mp(x,y));

                    numbFeedAround += feedAround[ind].first;
                    numbStuff1Around += stuff1Around[ind].first;
                    numbStuff2Around += stuff2Around[ind].first;
                }
            }
            sort(feedAround, feedAround + 9);
            sort(stuff1Around, stuff1Around + 9);
            sort(stuff2Around, stuff2Around + 9);
            // cout << "consume part" << endl;
            //consume part
            if(curCell.exist1) {
                ui64 needConsumeFeed = f.vConsumeFeed;
                ui64 curConsumeFeed;
                for(int i = 0;i < 9;++i) {
                    px = (line + feedAround[i].second.first + n) % n;
                    py = (k + feedAround[i].second.second + m) % m;
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
                        px = (line + stuff2Around[i].second.first + n) % n;
                        py = (k + stuff2Around[i].second.second + m) % m;
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
                            curCell.hungerRounds = 0;
                            --f.numbAlive1;
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
                    px = (line + stuff1Around[i].second.first + n) % n;
                    py = (k + stuff1Around[i].second.second + m) % m;
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
                    --f.numbAlive2;
                }
            }
            // cout << "divide and pollution part" << endl;
            //divide and pollution part
            if(curCell.stuff1 >= f.pollutionBound) {
                curCell.exist1 = 0;
                --f.numbAlive1;
            }

            if(curCell.stuff2 >= f.divisionBound
                && curCell.exist2 && numbFreeSpaceAround) {
                int gen = rand() % numbFreeSpaceAround;
                bool add = false;
                for(int x = -1;x <= 1; ++x) {
                    for(int y = -1; y <= 1; ++y) {
                        if(!x && !y) {
                            continue;
                        }

                        px = (line + x + n) % n;
                        py = (k + y + m) % m;

                        if(f.data[px][py].exist2)
                            continue;

                        if(!gen) {
                            f.data[px][py].exist2 = 1;
                            ++f.numbAlive2;
                            add = true;
                            break;
                        }
                        --gen;
                    }
                    if(add)
                        break;
                }
            }
            // cout << "produce part" << endl;
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
    sleep(1);
    system("clear");
    int n = f.n, m = f.m;
    for(ui32 i = 0;i < n;++i) {
        for(ui32 j = 0;j < m;++j) {
            ui32 sum = f.data[i][j].exist1 + 2*f.data[i][j].exist2;
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
        // cout << t << " OK" << endl;
        for(ui32 i = 0; i < n; ++i) {
            // cout << i << endl;
            calcLine(f, i, t);
        }
        visualize(f);

        cout << t + 1 << " " << f.numbAlive1 << " " <<  f.numbAlive2 << endl;
    }



    return f;
}

#endif
