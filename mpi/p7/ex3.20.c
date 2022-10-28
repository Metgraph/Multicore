#include <mpi.h>
#include <stdio.h>

void Get_Input(int my_rank, int comm_sz, double *a_p, double *b_p, int *n_p){
    int dest, position=0;
    char buff[128];
    if(my_rank==0){
        printf("Enter a, b, and n\n");
        scanf("%lf %lf %d", a_p, b_p, n_p);
        MPI_Pack(a_p, 1, MPI_DOUBLE, buff, 128, &position, MPI_COMM_WORLD);
        MPI_Pack(b_p, 1, MPI_DOUBLE, buff, 128, &position, MPI_COMM_WORLD);
        MPI_Pack(n_p, 1, MPI_INT, buff, 128, &position, MPI_COMM_WORLD);
    }
    MPI_Bcast(buff, 128, MPI_PACKED, 0, MPI_COMM_WORLD);
    if(my_rank!=0){
        position=0;
        MPI_Unpack(buff, 128, &position, a_p, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(buff, 128, &position, b_p, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(buff, 128, &position, n_p, 1, MPI_INT, MPI_COMM_WORLD);
    }
}

int main(){
    int my_rank, comm_sz;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    double a,b;
    int n;
    Get_Input(my_rank, comm_sz, &a, &b, &n);
    printf("Process %d: %lf, %lf, %d\n", my_rank, a, b, n);
    


    MPI_Finalize();

    return 0;
}