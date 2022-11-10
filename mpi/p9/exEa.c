/*
Write the odd-even sort algorithm using RMA with MPI_Fence
*/
#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int get_my_sz(int my_rank, int comm_sz, int v_sz) {
	return v_sz / comm_sz + (v_sz % comm_sz > my_rank ? 1 : 0);
}

int get_my_disp(int my_rank, int comm_sz, int v_sz) {
	int ret = v_sz % comm_sz;
	int div = v_sz / comm_sz;
	return ret > my_rank ? my_rank * (div + 1)
						 : ret * (div + 1) + div * (my_rank - ret);
}

int get_my_part(MPI_Win *win, int *my_buff, int my_sz, int my_disp) {
	MPI_Get(my_buff, my_sz, MPI_INT, 0, my_disp, my_sz, MPI_INT, *win);
}

void sort(int v[], int v_size) {
	for (int i = 0; i < v_size; i++) {
		int min = INT_MAX;
		int pos = 0;
		for (int j = i; j < v_size; j++) {
			if (v[j] < min) {
				min = v[j];
				pos = j;
			}
		}
		int temp = v[i];
		v[i] = min;
		v[pos] = temp;
	}
}

void Merge_low(int my_keys[], int recv_keys[], int temp_keys[], int local_m,
			   int local_r) {
	int m_i, r_i, t_i;

	m_i = r_i = t_i = 0;
	while (t_i < local_m) {
		if (my_keys[m_i] <= recv_keys[r_i] || r_i >= local_r) {
			temp_keys[t_i] = my_keys[m_i];
			t_i++;
			m_i++;
		} else {
			temp_keys[t_i] = recv_keys[r_i];
			t_i++;
			r_i++;
		}
	}

	for (m_i = 0; m_i < local_m; m_i++) {
		my_keys[m_i] = temp_keys[m_i];
	}
}

void Merge_high(int my_keys[], int recv_keys[], int temp_keys[], int local_m,
				int local_r) {
	int m_i, r_i, t_i;

	m_i = r_i = t_i = 0;
    m_i= local_m - 1;
    r_i= local_r - 1;
	while (t_i < local_m) {
		if (my_keys[m_i] >= recv_keys[r_i] || r_i < 0) {
			temp_keys[t_i] = my_keys[m_i];
			t_i++;
			m_i--;
		} else {
			temp_keys[t_i] = recv_keys[r_i];
			t_i++;
			r_i--;
		}
	}

	for (m_i = 0; m_i < local_m; m_i++) {
		my_keys[local_m-m_i-1] = temp_keys[m_i];
	}
}

void get_input(int *buff, int len, int disp) {
	for (int i = 0; i < len; i++) {
		buff[i + disp] = rand()%1000;
	}
}

void global_sort(MPI_Win *win, int *my_base, int my_rank, int comm_sz,
				 int my_sz, int v_sz) {
	int target_rank;
	int *recv_buff = malloc((my_sz + 1) * sizeof(int));
	int *temp_buff = malloc((my_sz) * sizeof(int));
	int next_r_sz, prev_r_sz;
	if (my_rank + 1 < comm_sz) {
		next_r_sz = get_my_sz(my_rank + 1, comm_sz, v_sz);
	}
	if (my_rank - 1 >= 0) {
		prev_r_sz = get_my_sz(my_rank - 1, comm_sz, v_sz);
	}
	for (int i = 0; i < comm_sz; i++) {
		if (i % 2 == 0) {
			if (my_rank % 2 == 0) {
				target_rank = my_rank + 1;
				if (target_rank < comm_sz) {
					MPI_Get(recv_buff, next_r_sz, MPI_INT, target_rank, 0,
							next_r_sz, MPI_INT, *win);
					MPI_Win_fence(0, *win);
					Merge_low(my_base, recv_buff, temp_buff, my_sz, next_r_sz);
				} else {
					MPI_Win_fence(0, *win);
				}
			} else {
				target_rank = my_rank - 1;
				if (target_rank >= 0) {
					MPI_Get(recv_buff, prev_r_sz, MPI_INT, target_rank, 0,
							prev_r_sz, MPI_INT, *win);
					MPI_Win_fence(0, *win);
					Merge_high(my_base, recv_buff, temp_buff, my_sz, prev_r_sz);
				} else {
					MPI_Win_fence(0, *win);
				}
			}
		} else {
			if (my_rank % 2 == 0) {
				target_rank = my_rank - 1;
				if (target_rank >= 0) {
					MPI_Get(recv_buff, prev_r_sz, MPI_INT, target_rank, 0,
							prev_r_sz, MPI_INT, *win);
					MPI_Win_fence(0, *win);
					Merge_high(my_base, recv_buff, temp_buff, my_sz, prev_r_sz);
				} else {
					MPI_Win_fence(0, *win);
				}
			} else {
				target_rank = my_rank + 1;
				if (target_rank < comm_sz) {
					MPI_Get(recv_buff, next_r_sz, MPI_INT, target_rank, 0,
							next_r_sz, MPI_INT, *win);
					MPI_Win_fence(0, *win);
					Merge_low(my_base, recv_buff, temp_buff, my_sz, next_r_sz);
				} else {
					MPI_Win_fence(0, *win);
				}
			}
		}
		MPI_Win_fence(0, *win);
	}
}

int main() {
#define V_SIZE 100
	int comm_sz, my_rank, *my_base;
	MPI_Win win;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Win_allocate(512 * sizeof(int), sizeof(int), MPI_INFO_NULL,
					 MPI_COMM_WORLD, &my_base, &win);

	int my_sz = get_my_sz(my_rank, comm_sz, V_SIZE);
	int my_disp =
		get_my_disp(my_rank, comm_sz, V_SIZE) + get_my_sz(0, comm_sz, V_SIZE);
	if (my_rank == 0) {
		get_input(my_base, V_SIZE, my_sz);
	}
	MPI_Win_fence(0, win);
	get_my_part(&win, my_base, my_sz, my_disp);
	sort(my_base, my_sz);
	MPI_Win_fence(0, win);
	global_sort(&win, my_base, my_rank, comm_sz, my_sz, V_SIZE);
	// printf("Process %d: first val=%d, last val=%d\n", my_rank, my_base[0],
	// 	   my_base[my_sz - 1]);

    MPI_Put(my_base, my_sz, MPI_INT, 0, my_disp, my_sz, MPI_INT, win);
    MPI_Win_fence(0, win);

    if(my_rank==0){
        printf("Result: ");
        for (int i = my_sz; i < V_SIZE+my_sz-1; i++)
        {
            printf("%d, ", my_base[i]);
        }
        printf("%d\n", my_base[V_SIZE-1+my_sz]);
        
    }

	MPI_Win_free(&win);
	MPI_Finalize();
	return 0;
}