#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <ctime>
using std::cout;
using std::endl;
using std::vector;

int main(int argc, char** argv) {
        long long  n = atoll(argv[1]);
        int th, rank;
 	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&th);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
        long long left = rank*(n/th), right = (rank + 1)*(n/th);
        if(rank + 1 == th) {
		right += n % th;
	}
	long  double pi = 0;
        for(long long i = left;i < right;++i) {
                pi += powl(-1,((2*i+1)%4 == 3))/((long double)(2*i + 1));
        }

	if(rank) {
		MPI_Send(&pi, 1, MPI_LONG_DOUBLE, 0, 13, MPI_COMM_WORLD);
	}

	long double next_part;
	if(!rank) {
		for(long long i = 1;i < th;++i) { 
                	MPI_Recv(&next_part, 1, MPI_LONG_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
        		pi += next_part;
		}
	}

	MPI_Finalize();
	if(!rank) {
        	pi *= 4;
		cout.precision(20);
        	cout << "pi ~ " << pi << endl;
	}
        return 0;
}
