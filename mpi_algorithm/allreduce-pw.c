#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#define WINDOW 64

int main(int argc, char **argv) {

  int size, rank;
  int *src=NULL, *dst=NULL;
  int *pw_dst=NULL, *tmp=NULL;
  int i, j;
  int send_peer, recv_peer;

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
  printf("Allreduce benchmark\n");

  /* allocate memory */
  src = (int *)malloc(max_size*sizeof(int));
  dst = (int *)malloc(max_size*sizeof(int));
  pw_dst = (int *)malloc(max_size*sizeof(int));
  tmp = (int *)malloc(max_size*sizeof(int));
  if (src == NULL || dst == NULL || pw_dst == NULL || tmp == NULL) {
     perror("cannot allocate memory\n");
     exit(0);
  }

  for (i=0; i < max_size; i++) {
      src[i] = rank*size + i;
      pw_dst[i] = 0; /* init */
      printf("rank=%d src index=%d has value %2d\n", rank, i, src[i]);
  }
     
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Allreduce (src, dst, max_size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  for (i=0; i < max_size; i++) {
      printf("rank=%d dst index=%d has value %2d\n", rank, i, dst[i]);
  }
  
  /* reduce-scatter phase */

  /* pairwise exchange */
  for(i=0;i<size;i++) {
	  send_peer = (rank ^ i);
	  recv_peer = (rank ^ i);
	  MPI_Isend(&src[send_peer*(max_size/size)], max_size/size, MPI_INT, send_peer, 1, MPI_COMM_WORLD, &send_request[i]);
	  MPI_Irecv(&tmp[recv_peer*(max_size/size)], max_size/size, MPI_INT, recv_peer, 1, MPI_COMM_WORLD, &recv_request[i]);
  }
  MPI_Waitall(size, send_request, status);
  MPI_Waitall(size, recv_request, status);


  /* reduction*/
  for(i=0;i<size;i++) {
	for(j=0;j<(max_size/size);j++) {
		pw_dst[rank*(max_size/size)+j] += tmp[i+(max_size/size)+j];
	}
  }

  /* allgather phase */
  for(i=0;i<size;i++) {
	send_peer = (rank ^ i);
	recv_peer = (rank ^ i);
	MPI_Isend(&pw_dst[rank*(max_size/size)], max_size/size, MPI_INT, send_peer, 1, MPI_COMM_WORLD, &send_request[i]);
	MPI_Irecv(&pw_dst[recv_peer*(max_size/size)], max_size/size, MPI_INT, recv_peer, 1, MPI_COMM_WORLD, &recv_request[i]);
  }
  MPI_Waitall(size, send_request, status);
  MPI_Waitall(size, recv_request, status);

 /* for (i=0; i < max_size; i++) {
      printf("rank=%d pw_dst index=%d has value %2d\n", rank, i, pw_dst[i]);
  } */
  
  if(memcmp(dst, pw_dst, max_size*sizeof(int)))
	printf("pairwise ERROR\n");
  MPI_Finalize();
  return 0;
}

