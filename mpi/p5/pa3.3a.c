/*
Write an MPI program that computes a tree-structured global sum.
First write your program for the special case in which comm_sz is a power of two.
*/

#include <mpi.h>
#include <stdio.h>

void tree_sum(int arr[], int len){
    for(int i=0; i<len; i+=2){
        arr[i/2]=arr[i]+arr[i+1];
    }
}

int main(){
    //8256
    int comm_sz, my_rank, total_sum;
    MPI_Init(NULL, NULL);
    //comm_sz must be power of 2
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    #define len_arr 128
    int global_arr[len_arr];
    for(int i=0; i<len_arr; i++){
        global_arr[i]=i+1;
    }

    int local_len=len_arr/comm_sz;
    int local_arr[local_len/2];
    while(local_len>1){
        tree_sum(local_arr, local_len);
        local_len/=2;
    }

    if(my_rank==0){
        total_sum=local_arr[0];
        for(int source=1; source<comm_sz; source++){
            MPI_Recv(local_arr, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_sum+=local_arr[0];
        }
        printf("The result is %d\n", total_sum);
    }else{
        MPI_Send(local_arr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    

    MPI_Finalize();
}