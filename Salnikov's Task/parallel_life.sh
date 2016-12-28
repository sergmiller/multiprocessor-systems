make -f Makefile.par
python transform_data.py
mpirun -np 4  ./parallel_life
