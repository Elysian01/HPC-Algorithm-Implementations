#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define WINDOW 64
int main(int argc, char **argv) {

  int size, rank;
  int send_peer, recv_peer;
  int *src=NULL, *dst=NULL;
  int *pw_dst=NULL, *rr_dst=NULL, *tf_dst=NULL;
  int i;
  MPI_Status status[WINDOW];
  MPI_Request send_request[WINDOW];
  MPI_Request recv_request[WINDOW];

  struct timeval start, end;
  float total_time=0;
  int max_size=0;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  max_size = size;
  printf("All2All benchmark\n");

  /* allocate memory */
  src = (int *)malloc(max_size*sizeof(int));
  dst = (int *)malloc(max_size*sizeof(int));
  pw_dst = (int *)malloc(max_size*sizeof(int));
  rr_dst = (int *)malloc(max_size*sizeof(int));
  tf_dst = (int *)malloc(max_size*sizeof(int));
  if (src == NULL || dst == NULL) {
     perror("cannot allocate memory\n");
     exit(0);
  }

  for (i=0; i < max_size; i++) {
      src[i] = i*size+rank;
      printf("rank=%d src index=%d has value %2d\n", rank, i, src[i]);
  }
     
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Alltoall (src, max_size/size, MPI_INT, dst, max_size/size, MPI_INT, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  /* linear exchange*/
  for(i=0;i<size;i++) {
    send_peer = (rank + i) % size;
    recv_peer = (rank - i + size) % size;
    MPI_Isend(&src[send_peer*(max_size/size)], max_size/size, MPI_INT, send_peer, 1, MPI_COMM_WORLD, &send_request[i]);
    MPI_Irecv(&rr_dst[recv_peer*(max_size/size)], max_size/size, MPI_INT, recv_peer, 1, MPI_COMM_WORLD, &recv_request[i]);
  }
  MPI_Waitall(size, send_request, status);
  MPI_Waitall(size, recv_request, status);

  /* pair-wise exchange */
  for(i=0;i<size;i++) {
    send_peer = rank ^ i;
    recv_peer = rank ^ i;
    MPI_Isend(&src[send_peer*(max_size/size)], max_size/size, MPI_INT, send_peer, 1, MPI_COMM_WORLD, &send_request[i]);
    MPI_Irecv(&pw_dst[recv_peer*(max_size/size)], max_size/size, MPI_INT, recv_peer, 1, MPI_COMM_WORLD, &recv_request[i]);
  }
  MPI_Waitall(size, send_request, status);
  MPI_Waitall(size, recv_request, status);

  /* torsten hoefler */
for(i=0;i<size;i++) {
    send_peer = (((i%2) + (rank%2))%2)*(size/2) + ((i/2) + (rank/2))%(size/2);
    recv_peer = send_peer;
    MPI_Isend(&src[send_peer*(max_size/size)], max_size/size, MPI_INT, send_peer, 1, MPI_COMM_WORLD, &send_request[i]);
    MPI_Irecv(&tf_dst[recv_peer*(max_size/size)], max_size/size, MPI_INT, recv_peer, 1, MPI_COMM_WORLD, &recv_request[i]);
  }
  MPI_Waitall(size, send_request, status);
  MPI_Waitall(size, recv_request, status);

  if(memcmp(dst, rr_dst, max_size*sizeof(int))) {
	 printf("LE ERROR\n");
  }
  if(memcmp(dst, pw_dst, max_size*sizeof(int))) {
	 printf("PW ERROR\n");
  }
  if(memcmp(dst, tf_dst, max_size*sizeof(int))) {
	 printf("TF ERROR\n");
  }
  for (i=0; i < max_size; i++) {
      printf("rank=%d dst index=%d has value %2d rr_dst %d\n", rank, i, dst[i], rr_dst[i]);
  }
  
  MPI_Finalize();
  return 0;
}

