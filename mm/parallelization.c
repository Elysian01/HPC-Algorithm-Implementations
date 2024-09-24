#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#define WIDTH (1024)
#define ITERATIONS (10)
#define TILE_SIZE (128)


int main() {
	float **P, **M, **N;
	int i, j, k, a;
	int x, y, z;
	float avg_time;
	
	struct timeval start, end;

	/* Memory Allocation */

	P = (float**)malloc(sizeof(float *)*WIDTH);	
	M = (float**)malloc(sizeof(float *)*WIDTH);	
	N = (float**)malloc(sizeof(float *)*WIDTH);

	for(int i=0;i<WIDTH;i++) {
		P[i] = (float *)malloc(sizeof(float)*WIDTH);
		M[i] = (float *)malloc(sizeof(float)*WIDTH);
		N[i] = (float *)malloc(sizeof(float)*WIDTH);
	}

	/*Checking if memory is NULL*/
	if (P == NULL || M == NULL || N == NULL) return 0;
	
	/*Initialization*/
	#pragma omp parallel for
	for(i=0;i<WIDTH;i++) {
		for(int j=0;j<WIDTH;j++) {
			P[i][j] = 0;
			M[i][j] = 1.5;
			N[i][j] = 1.8;
		}
	}

	/*Matrix Multiplication*/
	gettimeofday(&start, NULL);
	for(a=0;a<ITERATIONS;a++) {
		#pragma omp parallel for
		for(x=0;x<WIDTH;x+=TILE_SIZE) {
			for(y=0;y<WIDTH;y+=TILE_SIZE) {
				for(int z=0;z<WIDTH;z+=TILE_SIZE) {
					for(int i=x;i<x+TILE_SIZE;i++) {
						for(int k=z;k<z+TILE_SIZE;k++) {
							for(int j=y;j<y+TILE_SIZE;j++) {
								P[i][j] += M[i][j] * N[i][j];
							}
						}
					}
				}
			}
		}
	}

	gettimeofday(&end, NULL);
	avg_time = (float)((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec))/(float)ITERATIONS;
	
	printf("Matrix Multiplication time %f usecs\n", avg_time);

	/*Free Memory*/
	for(i=0;i<WIDTH;i++) {
		free(P[i]);
		free(M[i]);
		free(N[i]);
	}

	free(P); free(M); free(N);

	return 0;
}
