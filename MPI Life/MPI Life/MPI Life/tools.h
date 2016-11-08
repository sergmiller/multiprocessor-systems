//
//  tools.h
//  MPI Life
//
//  Created by Serg Miller on 08.11.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#ifndef tools_h
#define tools_h


#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cassert>

using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::pair;

pair<int,int> get_bounds(int r, int th, int n) {
    return std::make_pair(n/(th-1)*(r-1), n/(th-1)*r + (r == th-1) * (n%(th-1)));
}


void init_random_field(vector <vector <char> >& field,  int n, int m, float p) {
    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
            field[i][j] = (rand() <= ((float)RAND_MAX * p));
        }
    }
}

void test(vector <vector <char> >& a, vector <vector <char> >& b) {
    int n = (int)a.size();
    int m = (int)a[0].size();
    assert(a.size() == b.size());
    assert(a[0].size() == b[0].size());

    for(int i = 0;i < n;++i) {
        for(int j = 0;j < m;++j) {
                        assert(a[i][j] == b[i][j]);
        }
    }
    cout << "OK" << endl;
}

#endif /* tools_h */
