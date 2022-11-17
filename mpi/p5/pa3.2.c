/*
Suppose we toss darts randomly at a square dartboard, whose
bullseye is at the origin, and whose sides are 2 feet in length.
Suppose also that there’s a circle inscribed in the square dartboard.
The radius of the circle is 1 foot, and it’s area is  pi square feet.
If the points that are hit by the darts are uniformly distributed
(and we always hit the square), then the number of darts that hit
inside the circle should approximately satisfy the equation
            number in circle        pi
            -------------------- = ----
            total num of tosses     4
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

double rand_d(double b, double e) {
	int ri = rand();
	double r = (double)ri / (double)RAND_MAX;
	return r * (e - b) + b;
}

int main() {
	int comm_sz, my_rank, my_count = 0, total_count;
	const int total_attempts = 2048;
	double x, y, distance_square;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	const int my_attempts = total_attempts / comm_sz;

    //code suggested by track
	for (int i = 0; i < my_attempts; i++) {
		x = rand_d(-1.0, 1.0);
		y = rand_d(-1.0, 1.0);
		distance_square = x * x + y * y;
		if (distance_square <= 1.0) {
			my_count++;
		}
	}
    //end code suggestion

    //get the values excluded by distribution
	int my_rem = my_attempts * comm_sz + my_rank;
	if (my_rem < total_attempts) {
		x = rand_d(-1.0, 1.0);
		y = rand_d(-1.0, 1.0);
		distance_square = x * x + y * y;
		if (distance_square <= 1.0) {
			my_count++;
		}
	}

	if (my_rank == 0) {
		total_count = my_count;
		for (int source = 1; source < comm_sz; source++) {
			MPI_Recv(&my_count, 1, MPI_INT, source, 0, MPI_COMM_WORLD,
					 MPI_STATUS_IGNORE);
			total_count += my_count;
		}
		printf("%d of %d hit the circle, ratio is %lf\n", total_count,
			   total_attempts, (double)total_count / (double)total_attempts);
	} else {
		MPI_Send(&my_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}