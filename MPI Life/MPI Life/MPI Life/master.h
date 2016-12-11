//
//  master.h
//  MPI Life
//
//  Created by Serg Miller on 08.11.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#ifndef master_h
#define master_h

vector <vector<char> > MPI_Master(vector<vector<char> >& field, int n,int m, int tm, int th, int rank, MPI_Status* status) {
    vector<vector<char> > mpi_ans(n, vector<char>(m));

//    for(int i = 0;i < n;++i) {
//        for(int j = 0;j < m;++j) {
//            cout << (int)field[i][j] << " ";
//        }
//        cout << endl;
//    }
    pair<int,int> bounds;

    int prev,next;
    for(int i = 1; i < th;++i) {
        bounds = get_bounds(i,th,n);
//        cout << bounds.first << " " << bounds.second << endl;
        for(int j = bounds.first; j < bounds.second; ++j) {
            MPI_Send(&field[j][0], m, MPI_CHAR, i, i, MPI_COMM_WORLD);
        }

        prev = (bounds.first - 1 + n) % n;
        next = bounds.second%n;
        MPI_Send(&field[prev][0], m, MPI_CHAR, i, 1, MPI_COMM_WORLD);
        MPI_Send(&field[next][0], m, MPI_CHAR, i, 2, MPI_COMM_WORLD);
    }

    for(int i = 1;i < th;++i) {
        bounds = get_bounds(i,th,n);
        for(int j = bounds.first; j < bounds.second; ++j) {
            MPI_Recv(&mpi_ans[j][0], m, MPI_CHAR, i, 5, MPI_COMM_WORLD, status);
        }
    }

    return mpi_ans;
}

#endif /* master_h */
