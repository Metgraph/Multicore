/*
Write an MPI program that computes a tree-structured global sum.
First write your program for the special case in which comm_sz is a power of two.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int sum(int arr[], int len){
    int res=0;
    for (int i = 0; i < len; i++)
    {
        res+=arr[i];
    }
    return res;
    
}

int main(){
    int comm_sz, my_rank, total_sum, *comm_arr, *my_arr;
    MPI_Init(NULL, NULL);
    //comm_sz must be power of 2
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    #define LEN_ARR 128

    if(LEN_ARR%comm_sz!=0){
        if(my_rank==0){
            fprintf(stderr, "ERROR: wrong process number, it must be a power of 2\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // int comm_arr[len_arr];
    comm_arr=malloc(LEN_ARR*sizeof(int));

    for(int i=0; i<LEN_ARR; i++){
        comm_arr[i]=1;
    }
    int my_len=LEN_ARR/comm_sz;
    // int my_arr[my_len];
    my_arr=malloc(my_len*sizeof(int));

    if(my_rank==0){
        total_sum=my_arr[0];
        for(int source=1; source<comm_sz; source++){
            MPI_Send(&comm_arr[my_len*source], my_len, MPI_INT, source, 0, MPI_COMM_WORLD);
            total_sum+=my_arr[0];
        }
        for (int i = 0; i < my_len; i++)
        {
            my_arr[i]=comm_arr[i];
        }
        
    }else{
        MPI_Recv(my_arr, my_len, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int my_res=sum(my_arr, my_len);
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
    
    
    free(comm_arr);
    MPI_Finalize();
}