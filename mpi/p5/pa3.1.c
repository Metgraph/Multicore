/*
Use MPI to implement the histogram program.
Have process 0 read in the input data and distribute it among
the processes.
Also have process 0 print out the histogram.
*/

#include <stdio.h>
#include <mpi.h>
#include <string.h>

void sum_v5(int source[], int dest[]){
    for(int i=0; i<5;i++){
        dest[i]+=source[i];
    }
}

int main(){
    //0-1:6, 1-2:3, 2-3:2, 3-4: 3, 4-5:6
    float global_arr[]={1.3,2.9,0.4,0.3,1.3,4.4,1.7,0.4,3.2,0.3,4.9,2.4,3.1,4.4,3.9,0.4,4.2,4.5,4.9,0.9};
    int total_count[5];
    int my_count[]={0,0,0,0,0};
    int len_arr=sizeof(global_arr)/sizeof(float);
    int comm_sz, my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int local_len=len_arr/comm_sz;
    int a = local_len*my_rank;
    int b = a+local_len;
    for(int i=a; i<b; i++){
        int num=(int)(global_arr[i]);
        my_count[num]++;
    }

    int my_rem = local_len*comm_sz+my_rank;
    if(my_rem<len_arr){
        my_count[(int)global_arr[my_rem]]++;
    }

    if(my_rank==0){
        memcpy(total_count, my_count, 5*sizeof(int));
        for (int source = 1; source < comm_sz; source++)
        {
            MPI_Recv(my_count, 5, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum_v5(my_count, total_count);
        }
        printf("Risultati: 0-1: %d, 1-2: %d, 2-3: %d, 3-4: %d, 4-5: %d\n", total_count[0], total_count[1], total_count[2], total_count[3], total_count[4]);
    }else{
        MPI_Send(my_count, 5, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }



    MPI_Finalize();
}