/*
Use MPI to implement the histogram program.
Have process 0 read in the input data and distribute it among
the processes.
Also have process 0 print out the histogram.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// 0-1:6, 1-2:3, 2-3:2, 3-4: 3, 4-5:6
float comm_arr[] = {1.3, 2.9, 0.4, 0.3, 1.3, 4.4, 1.7, 0.4, 3.2, 0.3,
					4.9, 2.4, 3.1, 4.4, 3.9, 0.4, 4.2, 4.5, 4.9, 0.9};
int comm_len = sizeof(comm_arr) / sizeof(float);
int thread_count, comm_v5[5];
pthread_mutex_t mutex;

void* Thread_sum(void* rank) {
	long my_rank = (long)rank;
	int my_v5[5] = {0};

	// get the index of first element to analyze
	int my_len = comm_len / thread_count;
	int my_part = my_len * my_rank;

	// calculate the histogram
	int v5_ind;
	for (int i = my_part; i < my_part + my_len; i++) {
		int v5_ind = (int)comm_arr[i];
		my_v5[v5_ind]++;
	}
	// add in the histogram values excluded by the division
	if (my_rank < comm_len % thread_count) {
		v5_ind = (int)comm_arr[my_len * thread_count + my_rank];
		my_v5[v5_ind]++;
	}

	// put own result in global result
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < 5; i++) {
		comm_v5[i] += my_v5[i];
	}

	pthread_mutex_unlock(&mutex);
	return NULL;
}

int check_args(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "The number of thread must be passed: pa4.1 n_thread\n");
		return 1;
	}
	thread_count = strtol(argv[1], NULL, 10);
	if (thread_count < 1) {
		printf("The number of thread must be greater than 0\n");
		return 1;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	long thread;
	pthread_t* thread_handles;

	if (check_args(argc, argv) != 0) {
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		comm_v5[i] = 0;
	}

	pthread_mutex_init(&mutex, NULL);
	thread_handles = malloc(thread_count * sizeof(pthread_t));

	for (thread = 1; thread < thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, Thread_sum,
					   (void*)thread);
	}

	Thread_sum(0);

	for (thread = 1; thread < thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}

	printf("Results: ");
	for (int i = 0; i < 5 - 1; i++) {
		printf("%d-%d: %d, ", i, i + 1, comm_v5[i]);
	}
	printf("%d-%d: %d\n", 4, 4 + 1, comm_v5[4]);

	pthread_mutex_destroy(&mutex);
	free(thread_handles);
	return 0;
}