//
//  slave.h
//  MPI Life
//
//  Created by Serg Miller on 08.11.16.
//  Copyright © 2016 sergmiller. All rights reserved.
//

#ifndef slave_h
#define slave_h

void MPI_Slave(int n,int m, int tm, int th, int rank, MPI_Status* status) {
    vector <vector <char> > field_part[2];
    pair <int,int> bounds = get_bounds(rank, th, n);

    field_part[0].resize(bounds.second - bounds.first + 2, vector <char>(m));
    field_part[1].resize(bounds.second - bounds.first + 2, vector <char>(m));

    for(int i = bounds.first; i < bounds.second; ++i) {
        MPI_Recv(&field_part[0][i-bounds.first + 1][0], m, MPI_CHAR, 0, rank, MPI_COMM_WORLD, status);
    }

    MPI_Recv(&field_part[0][0][0], m, MPI_CHAR, 0, 1, MPI_COMM_WORLD, status);
    MPI_Recv(&field_part[0][bounds.second - bounds.first + 1][0], m, MPI_CHAR, 0, 2, MPI_COMM_WORLD, status);

    int cur_part = 0;
    int prev, next;
    prev = rank - 1;
    next = rank + 1;
    if(rank == 1) {
        prev = th - 1;
    }
    if(rank == th - 1) {
        next = 1;
    }


    // vector <char> buffer_isend[2];
    // vector <char> buffer_irecv[2];

    for(int t = 0; t < tm; ++t) {
        for(int i = 1; i <= bounds.second - bounds.first; ++i) {
            calc_next(&field_part[cur_part], &field_part[1-cur_part],
              i, bounds.second - bounds.first + 2, m);
        }
        cur_part = 1 - cur_part;
        //Send first line to left neighboor
        MPI_Send(&field_part[cur_part][1][0],
            m, MPI_CHAR, prev, 3, MPI_COMM_WORLD);

        //Send last line to tight neighboor
        MPI_Send(&field_part[cur_part][bounds.second - bounds.first][0],
            m, MPI_CHAR, next, 4, MPI_COMM_WORLD);

        //Revieve last line from left neighboor
        MPI_Recv(&field_part[cur_part][0][0],
            m, MPI_CHAR, prev, 4, MPI_COMM_WORLD, status);

        //Revieve first line from right neighboor
        MPI_Recv(&field_part[cur_part][bounds.second - bounds.first + 1][0],
            m, MPI_CHAR, next, 3, MPI_COMM_WORLD, status);
    }

    for(int i = 1; i <= bounds.second - bounds.first; ++i) {
        MPI_Send(&field_part[cur_part][i][0], m, MPI_CHAR, 0, 5, MPI_COMM_WORLD);
    }
}

#endif /* slave_h */
