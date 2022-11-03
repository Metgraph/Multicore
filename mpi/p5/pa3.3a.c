/*
Write an MPI program that computes a tree-structured global sum.
First write your program for the special case in which comm_sz is a power of two.
*/

#include <mpi.h>
#include <stdio.h>


int sum(int arr[], int len){
    int res=0;
    for (int i = 0; i < len; i++)
    {
        res+=arr[i];
    }
    return res;
    
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
        global_arr[i]=1;
    }
    int local_len=len_arr/comm_sz;
    int local_arr[local_len];

    if(my_rank==0){
        total_sum=local_arr[0];
        for(int source=1; source<comm_sz; source++){
            MPI_Send(&global_arr[local_len*source], local_len, MPI_INT, source, 0, MPI_COMM_WORLD);
            total_sum+=local_arr[0];
        }
        for (int i = 0; i < local_len; i++)
        {
            local_arr[i]=global_arr[i];
        }
        
    }else{
        MPI_Recv(local_arr, local_len, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int my_res=sum(local_arr, local_len);
    int buff;
    for (int i = 1; i < comm_sz; i*=2)
    {
        if(my_rank%(i*2)==0){
            MPI_Recv(&buff, 1, MPI_INT, my_rank+i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            my_res+=buff;
        }else if((my_rank-i)%(i*2)==0){
            MPI_Send(&my_res, 1, MPI_INT, my_rank-i, 0, MPI_COMM_WORLD);
            break;
        }
    }

    if(my_rank==0){
        printf("Sum result is %d\n", my_res);
    }
    
    

    MPI_Finalize();
}