#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char **argv) {
	int size, rank;
	int *src=NULL, *dst=NULL;
	int i;

	MPI_Status status;
	struct timeval start, end;
	int max_size = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	/*warm up*/
	/* src=(int *)malloc(max_size*sizeof(int));
	dst=(int *)malloc(max_size*sizeof(int));

	if(src==NULL || dst==NULL) {
		perror("cannot allocate memory\n");
		exit(0);
	}

	for(i=0;i<max_size;i++){
		src[i]=rank;
		printf("rank=%d src index=%d has value= %2d\n", rank, i, src[i]);
	}*/

	/* warm up */
	for(int size=1;size<=MAX_SIZE; size*=2) {
		for(int i=0;i< WARMUP; i++) {
			if (rank == src_rank) {
				MPI_Send(src, size, MPI_CHAR, dst_rank, 1, MPI_COMM_WORLD);
				MPI_Recv(dst, size, MPI_CHAR, dst_rank, 1, MPI_COMM_WORLD, &status);
			}
			else if (rank == dst_rank) {
				MPI_Recv(dst, size, MPI_CHAR, src_rank, 1, MPI_COMM_WORLD, &status);
				MPI_Send(src, size, MPI_CHAR, src_rank, 1, MPI_COMM_WORLD);
			}
		}
	}

	/* Benchmark */

	MPI_Finalize();
	return 0;
}
