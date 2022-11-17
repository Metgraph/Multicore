/*
Create a program that calculates an approximation of
integral on f using the trapezoidal rule. The program
must handle comm_sz not evenly divided by the number of areas
*/

#include <mpi.h>
#include <stdio.h>

//a function x->y
double f(double x){
    return x*x;
}

//calculate trapezoids area between left_endpt and right_endpt
double Trap(double left_endpt, double right_endpt, int trap_count, double base_len){
    double estimate, x;
    estimate=(f(left_endpt) + f(right_endpt))/2.0;
    for (int i = 1; i <= trap_count; i++)
    {
        x = left_endpt + i*base_len;
        estimate+=f(x);
    }
    estimate = estimate*base_len;
    return estimate;
}

int main(){
    int my_rank, comm_sz, n=1027, local_n;
    double a=0.0, b=5.0, h, my_a, my_b;
    double my_area, total_area;
    int source;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    h=(b-a)/n;
    local_n=n/comm_sz;

    //calculate my left point
    my_a=a+my_rank*local_n*h;
    //calculate my right point
    my_b=my_a+local_n*h;

    my_area=Trap(my_a, my_b, local_n, h);

    //get remaining trapezoid excluded by distribution
    if(my_rank<n%comm_sz){
        double local_a2=a+(n-(n%comm_sz)+my_rank)*h;
        my_area+=(f(local_a2) + f(local_a2+h))/2.0*h;
    }

    //There should be Gather
    if(my_rank!=0){
        MPI_Send(&my_area, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }else{
        total_area = my_area;
        for(source = 1; source < comm_sz; source++){
            MPI_Recv(&my_area, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_area+=my_area;
        }
    }

    if(my_rank==0){
        printf("With n = %d trapezoids, our estimate\n", n);
        printf("of the integral from %f to %f = %.15e\n", a,b, total_area);
    }
    MPI_Finalize();
    return 0;
}