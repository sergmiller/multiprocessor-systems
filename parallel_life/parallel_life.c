#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define TM 2

struct field{
    int step;
    int size;
    int qnt;
    char* data[2];
};

typedef struct field field_t;

int dirs[8][2] = {{0,1}, {1,0}, {-1,0}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

void init_field_with_data(field_t* field, int _size, int _qnt, int* x, int* y) {
    field->step = 0;
    field->size = _size;
    field->qnt = _qnt;
    field->data[0] = (char*)calloc(_size * _size, sizeof(char));
    field->data[1] = (char*)malloc(_size * _size * sizeof(char));
    for(int i = 0;i < _qnt;++i) {
        *(field->data[0] + x[i] * _size + y[i]) = 1;
    }
}

void init_field_with_rand(field_t* field, int _size) {
    srand(time(NULL));
    field->step = 0;
    field->size = _size;
    field->qnt = 0;
    field->data[0] = (char*)calloc(_size * _size, sizeof(char));
    field->data[1] = (char*)malloc(_size * _size * sizeof(char));
    int cur;
    for(int i = 0;i < _size; ++i) {
        for(int j = 0;j < _size;++j) {
            cur = rand()%2;
            if(cur) {
             *(field->data[0] + i*_size + j) = 1;
             ++field->qnt;
            }
        }
    }
}

void calc_next(field_t* field) {
    int size = field->size;
    int cur_sum, nx, ny;

    ++field->step;

    if(!field->qnt) {
        memset(field->data[field->step & 1], 0, size * size * sizeof(char));
        return;
    }

    char* prev_data = field->data[1 - field->step & 1];
    char* cur_data = field->data[field->step & 1];

    for(int i = 0; i < size; ++i) {
        for(int j = 0; j < size; ++j) {
            cur_sum = 0;

            for(int k = 0; k < 8; ++k) {
                nx = (i + dirs[k][0] + size) % size;
                ny = (j + dirs[k][1] + size) % size;
                if(*(prev_data + size * nx + ny)) {
                    ++cur_sum;
                }
            }

            if((!*(prev_data + size * i + j) && cur_sum == 3) || (*(prev_data + size * i + j) && (cur_sum == 2 || cur_sum == 3))) {
                 *(cur_data + size * i + j) = 1;
            } else {
                 *(cur_data + size * i + j) = 0;
            }
        }
    }
}

void visualize(field_t* field) {
    system("clear");
    int size = field->size;
    char* cur_data = field->data[field->step & 1];
    for(int i = 0;i < size;++i) {
        for(int j = 0;j < size;++j) {
            if(*(cur_data + size * i + j)) {
                printf("#");
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
}

void life(int argc, char** argv) {
    assert(argc > 3);
    int size = atoi(argv[1]);
    int steps = atoi(argv[2]);
    int do_draw = atoi(argv[3]);
    int start_alive = atoi(argv[4]);
    field_t field;
    if(start_alive == -1) {
        init_field_with_rand(&field, size);
    } else {
        assert(argc > 4 + 2*start_alive);        
        int* x = (int*)malloc(start_alive * sizeof(int));
        int* y = (int*)malloc(start_alive * sizeof(int));
        for(int i = 0;i < start_alive; ++i) {
            x[i] = atoi(argv[2*i + 5]);
            y[i] = atoi(argv[2*i + 6]);
        }
        init_field_with_data(&field, size, start_alive, x, y);
    }

    visualize(&field);
    sleep(TM);

    int start_time = clock();
    
    while(field.step < steps) {
        calc_next(&field);
        visualize(&field);
        sleep(TM);
    }

    int times_per_sec = (clock() - start_time)/1000;
    printf("/nsteps: %d, work time:%d/n", steps, times_per_sec);
}

int main(int argc, char** argv) {
    life(argc, argv);
    // char c = '#';
    // printf("%c\n",c);
    // printf("%c\n",c-1);
    return 0;
}