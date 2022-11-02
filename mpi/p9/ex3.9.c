/*
Write an MPI program that implements multiplication of a vector by
a scalar and dot product. The user should enter two vectors and a
scalar, all of which are read in by process 0 and distributed among
the processes.
The results are calculated and collected onto process 0, which prints
them.
The program must use RMA and must handle vectors that are not
perfectly divisible by the number of process.
*/

#include <mpi.h>
#include <stdio.h>

void get_inputs(double v1[], double v2[], double *scalar, int v_sz) {
	for (int i = 0; i < v_sz; i++) {
		printf("Insert a number to place in the first vector in position %d\n",
			   i);
		scanf("%lf", &v1[i]);
	}
	for (int i = 0; i < v_sz; i++) {
		printf("Insert a number to place in the second vector in position %d\n",
			   i);
		scanf("%lf", &v2[i]);
	}
	printf("Insert the scalar\n");
	scanf("%lf", scalar);
	// for (int i = 0; i < v_sz; i++) {
	// 	v1[i] = i + 1;
	// 	v2[v_sz-i-1] = i + 1;
	// }
	// *scalar = 10;
}

void print_vector(double v[], int v_sz) {
	if (v_sz == 0) {
		printf("[]\n");
	} else {
		printf("[");
		for (int i = 0; i < v_sz - 1; i++) {
			printf("%lf, ", v[i]);
		}
		printf("%lf]\n", v[v_sz - 1]);
	}
}

void scatter_rma(MPI_Win *win, int comm_sz, double v1[], double v2[],
				 int v_size, double scalar, int win_sz) {
	int v_diff = 0;

	for (int i = 0; i < comm_sz; i++) {
		// check if target_count should change with different target_disp

		MPI_Put(&scalar, 1, MPI_DOUBLE, i, 0, 1, MPI_DOUBLE, *win);
		int i_v_size = v_size / comm_sz + ((v_size % comm_sz) > i);
		MPI_Put(&v1[v_diff], i_v_size, MPI_DOUBLE, i, 1, i_v_size, MPI_DOUBLE,
				*win);
		MPI_Put(&v2[v_diff], i_v_size, MPI_DOUBLE, i, 1 + i_v_size, i_v_size,
				MPI_DOUBLE, *win);

		v_diff += i_v_size;
	}
}

int get_disp_v(int my_rank, int comm_sz, int gv_sz) {
	int lv_size = gv_sz / comm_sz;
	int ret = gv_sz % comm_sz;
	if (ret > my_rank) {
		return my_rank * (lv_size + 1);
	} else {
		return ret * (lv_size + 1) + (my_rank - ret) * lv_size;
	}
}

void process_data(MPI_Win *win, int my_rank, int ldisp_v, int lv_sz, int gv_sz,
				  int buff_bg, double my_base[]) {
	// curr_disp space already taken from local data
	int curr_disp = buff_bg * 2 + 1;
	// int lv1[lv_sz], lv2[lv_sz], ls = 19;
	double ld = 0;

	for (int i = 1; i < lv_sz + 1; i++) {
		ld += my_base[i] * my_base[i + lv_sz];
		my_base[i] *= my_base[0];
		my_base[i + lv_sz] *= my_base[0];
	}
	MPI_Accumulate(&ld, 1, MPI_DOUBLE, 0, curr_disp, 1, MPI_DOUBLE, MPI_SUM,
				   *win);
	curr_disp += 1 + ldisp_v;
	MPI_Put(&my_base[1], lv_sz, MPI_DOUBLE, 0, curr_disp, lv_sz, MPI_DOUBLE,
			*win);
	curr_disp += gv_sz;
	MPI_Put(&my_base[1 + lv_sz], lv_sz, MPI_DOUBLE, 0, curr_disp, lv_sz,
			MPI_DOUBLE, *win);
}

int main() {
	int comm_sz, my_rank;
#define V_SIZE 10
	double global_v1[V_SIZE], global_v2[V_SIZE], global_scalar;
	MPI_Win win;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	double *my_base;

	MPI_Win_allocate(1024 * sizeof(double), sizeof(double), MPI_INFO_NULL,
					 MPI_COMM_WORLD, &my_base, &win);
	MPI_Win_fence(0, win);

	// get inputs and give a portion of them to each process
	if (my_rank == 0) {
		get_inputs(global_v1, global_v2, &global_scalar, V_SIZE);
		scatter_rma(&win, comm_sz, global_v1, global_v2, V_SIZE, global_scalar,
					1024);
	}

	// wait that all processes get their data
	MPI_Win_fence(0, win);

	// calculate own data length
	int local_v_sz = V_SIZE / comm_sz + (V_SIZE % comm_sz > my_rank);
	int comm_buff_begin = V_SIZE / comm_sz + (V_SIZE % comm_sz > 0);
	// calculate own local displacement in the result vector
	int my_disp = get_disp_v(my_rank, comm_sz, V_SIZE);
	process_data(&win, my_rank, my_disp, local_v_sz, V_SIZE, comm_buff_begin,
				 my_base);
	// wait all processes send their data
	MPI_Win_fence(0, win);

	if (my_rank == 0) {
		int curr_pos = comm_buff_begin * 2 + 1;
		printf("The product for scalar of first vector is ");
		print_vector(&my_base[curr_pos+1], V_SIZE);
		printf("The product for scalar of second vector is ");
		print_vector(&my_base[V_SIZE + curr_pos+1], V_SIZE);
		printf("The dot product of vectors is %lf\n", my_base[curr_pos]);
	}

	MPI_Win_free(&win);
	MPI_Finalize();
	return 0;
}