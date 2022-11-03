/*
Write an MPI program that computes a tree-structured global sum.
Write your program for any case.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int sum(int arr[], int len) {
	int res = 0;
	for (int i = 0; i < len; i++) {
		res += arr[i];
	}
	return res;
}

int main() {
	int comm_sz, my_rank, total_sum;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

#define len_arr 119

	int global_arr[len_arr];
	for (int i = 0; i < len_arr; i++) {
		global_arr[i] = 1;
	}
	int local_len = len_arr / comm_sz + (len_arr % comm_sz > my_rank);
	int *local_arr = malloc(local_len * sizeof(int));

	if (my_rank == 0) {
		int base_len = len_arr / comm_sz;
		int ret_len = len_arr % comm_sz;
		total_sum = local_arr[0];
        int add_v=0;
		for (int source = 1; source < comm_sz; source++) {
            add_v+= (ret_len>source-1);
			MPI_Send(&global_arr[base_len * source + add_v],
					 base_len+(ret_len>source), MPI_INT, source, 0, MPI_COMM_WORLD);
			total_sum += local_arr[0];
		}
		for (int i = 0; i < local_len; i++) {
			local_arr[i] = global_arr[i];
		}

	} else {
		MPI_Recv(local_arr, local_len, MPI_INT, 0, 0, MPI_COMM_WORLD,
				 MPI_STATUS_IGNORE);
	}

	int my_res = sum(local_arr, local_len);
	int buff;
	for (int i = 1; i < comm_sz; i *= 2) {
		if (my_rank % (i * 2) == 0) {
			if (comm_sz > my_rank + i) {
				MPI_Recv(&buff, 1, MPI_INT, my_rank + i, 0, MPI_COMM_WORLD,
						 MPI_STATUS_IGNORE);
				my_res += buff;
			}
		} else if ((my_rank - i) % (i * 2) == 0) {
			MPI_Send(&my_res, 1, MPI_INT, my_rank - i, 0, MPI_COMM_WORLD);
			break;
		}
	}

	if (my_rank == 0) {
		printf("Sum result is %d\n", my_res);
	}

	free(local_arr);

	MPI_Finalize();
}