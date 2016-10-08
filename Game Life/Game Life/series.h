//
//  series.h
//  Game Life
//
//  Created by Serg Miller on 08.10.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#ifndef series_h
#define series_h

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>

using std::vector;
using std::cin;
using std::cout;
using std::endl;


void calc_next(vector <vector <char> >& cur, vector <vector <char> >& next,int i,int n, int m) {
    int sum, px, py;
    for(int j = 0;j < m;++j) {
        sum = 0;
        for(int x = -1; x<= 1; ++x) {
            for(int y = -1;y <= 1; ++y) {
                if(!x && !y) {
                    continue;
                }
                px = (i + x + n) % n;
                py = (j + y + m) % m;
                sum += cur[px][py];
            }
        }
        if((!cur[i][j] && sum == 3) || (cur[i][j] && (sum == 2))) {
            next[i][j] = 1;
        } else {
            next[i][j] = 0;
        }
    }
}


vector <vector <char> >  life_s_simple(vector <vector <char> > & data, int n, int m, int tm) {
    vector <vector <char> > field [2];
    
    field[0] = data;
    field[1].resize(n,vector<char>(m));
    
    for(int t = 0; t < tm;++t) {
        int cur = t & 1;
        int next = 1 - cur;
        for(int i = 0;i < n;++i) {
            calc_next(field[cur], field[next], i, n, m);
        }
    }
    
    cout << "out" << endl;
    return field[tm & 1];
}

#endif /* series_h */
