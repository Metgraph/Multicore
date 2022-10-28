/*
Use Gatherv and Scatterv
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	int comm_sz, my_rank;
#define V_SIZE 13
    int global_v[V_SIZE];
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	if (my_rank == 0) {
		for (int i = 0; i < V_SIZE; i++) {
			// printf("Insert a number\n");
			// scanf("%d", &global_v[i]);
            global_v[i]=i+1;
		}
	}
    // int size_lv = V_SIZE/comm_sz + (V_SIZE%comm_sz>0);
    int size_lv = V_SIZE/comm_sz;
    int buff_size = size_lv+(V_SIZE%comm_sz>0);
    int counts[comm_sz];//={size_lv};
    int displs[comm_sz];
    int added=0;
    int next_displs= 0;
    for (int i = 0; i < comm_sz; i++)
    {
        displs[i]=next_displs;
        next_displs+=(size_lv+(V_SIZE%comm_sz>i));
        counts[i]=size_lv+(V_SIZE%comm_sz>i);
    }
    
    
    
    int my_vsz=counts[my_rank];
    int local_v[my_vsz];
    MPI_Scatterv(global_v, counts, displs, MPI_INT, local_v, my_vsz, MPI_INT, 0, MPI_COMM_WORLD);
    for(int i=0; i<(my_vsz/2); i++){
        int temp=local_v[my_vsz-i-1];
        local_v[my_vsz-i-1]=local_v[i];
        local_v[i]=temp;
    }

    MPI_Gatherv(local_v, my_vsz, MPI_INT, global_v, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if(my_rank==0){
        for (int i = 0; i < V_SIZE; i++)
        {
            printf("%d\n", global_v[i]);
        }
        
    }
    

	MPI_Finalize();
	return 0;
}