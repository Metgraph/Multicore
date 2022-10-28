/*
Print in order outputs of processes according to their ranks
*/

#include <stdio.h>
#include <mpi.h>

int main(){
    char buff[64];
    int my_rank, comm_sz;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if(my_rank==0){
        printf("Proc %d of %d\n", my_rank, comm_sz);
        for(int i=1; i<comm_sz; i++){
            MPI_Recv(buff, 64, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%s", buff);
        }
    }else{
        int len = sprintf(buff, "Proc %d of %d\n", my_rank, comm_sz);
        MPI_Send(buff, len+1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }


    MPI_Finalize();
    return 0;
}