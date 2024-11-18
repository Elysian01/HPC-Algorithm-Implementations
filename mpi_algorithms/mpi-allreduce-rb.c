#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
#define WINDOW 64

int main(int argc, char **argv) {
    int size, rank;
    int *src = NULL, *dst = NULL;
    int *rb_dst = NULL, *tmp = NULL;
    int peer;
    int i, j;
    MPI_Status status[2];
    MPI_Request request[2];
    struct timeval start, end;
    float total_time = 0;
    int max_size = 0;
    int msg_size;
    int stages;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    max_size = size;

    printf("Allreduce benchmark\n");

    src = (int *)malloc(max_size * sizeof(int));
    dst = (int *)malloc(max_size * sizeof(int));
    rb_dst = (int *)malloc(max_size * sizeof(int));
    tmp = (int *)malloc(max_size * sizeof(int));

    if (src == NULL || dst == NULL || rb_dst == NULL || tmp == NULL) {
        perror("Memory error\n");
        exit(0);
    }

    for (i = 0; i < max_size; i++) {
        src[i] = size + i * 2;
        rb_dst[i] = 0;
        printf("rank=%d src index=%d has value %2d\n", rank, i, src[i]);
    }

    MPI_Allreduce(src, dst, max_size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    memcpy(rb_dst, src, max_size * sizeof(int));
    stages = log2(size);
    int *buffer_ptr = rb_dst;

    for (i = 0; i < stages; i++) {
        msg_size = max_size / (int)pow(2,i+1);
        peer = (rank ^ (int)pow(2,i));

        if (rank < peer) {
            MPI_Isend(buffer_ptr + msg_size, msg_size, MPI_INT, peer, 2,
                      MPI_COMM_WORLD, &request[0]);
        } else {
            MPI_Isend(buffer_ptr, msg_size, MPI_INT, peer, 2,
                      MPI_COMM_WORLD, &request[0]);
            buffer_ptr += msg_size;
        }

        MPI_Irecv(tmp, msg_size, MPI_INT, peer, 2,
                  MPI_COMM_WORLD, &request[1]);

        MPI_Waitall(2, request, status);

        for (j = 0; j < msg_size; j++) {
            buffer_ptr[j] += tmp[j];
        }
    }

    for (i = stages - 1; i >= 0; i--) {
        msg_size = max_size / (int)pow(2,i+1);
        peer = (rank ^ (int)pow(2,i));

        MPI_Isend(buffer_ptr, msg_size, MPI_INT, peer, 3,
                  MPI_COMM_WORLD, &request[1]);

        if (rank < peer) {
            MPI_Irecv(buffer_ptr + msg_size, msg_size, MPI_INT, peer, 3,
                      MPI_COMM_WORLD, &request[0]);
        } else {
            MPI_Irecv(buffer_ptr - msg_size, msg_size, MPI_INT, peer, 3,
                      MPI_COMM_WORLD, &request[0]);
            buffer_ptr -= msg_size;
        }

        MPI_Waitall(2, request, status);
    }

    if(memcmp(dst, rb_dst, max_size*sizeof(int)))
        printf("rb algo ERROR\n");
    else printf("rb algo success\n");
    MPI_Finalize();
    return 0;
}