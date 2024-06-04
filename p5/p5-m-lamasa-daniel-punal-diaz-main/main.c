#include <stdio.h>
#include <math.h>
#include "mpi.h"

int main(int argc, char *argv[]){

    int numprocs, rank;
    double aux;
    MPI_Init(&argc, &argv);

    MPI_Comm_size ( MPI_COMM_WORLD , &numprocs);
    MPI_Comm_rank ( MPI_COMM_WORLD , &rank);

    while(1) {
        aux = 0.0;
        if (rank != 0) {
            int n_rec;
            MPI_Bcast(&n_rec, 1, MPI_INT, 0, MPI_COMM_WORLD);

            if(n_rec == 0) break;       //para salir de bucle

            double h = 1.0 / (double) n_rec;
            double sum = 0.0;
            double x;
            for (int i = rank + 1; i <= n_rec; i += numprocs) {
                x = h * ((double) i - 0.5);
                sum += 4.0 / (1.0 + x * x);
            }

            MPI_Reduce(&sum, &aux, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        } else {

            int n;
            double pi, h, sum, x;

            printf("Enter the number of intervals: (0 quits) \n");
            scanf("%d", &n);

            MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

            if (n == 0) break;  //para salir del bucle

            h = 1.0 / (double) n;
            sum = 0.0;

            for (int j = rank + 1; j <= n; j += numprocs) {
                x = h * ((double) j - 0.5);
                sum += 4.0 / (1.0 + x * x);
            }

            MPI_Reduce(&sum, &aux, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

            double PI25DT = 3.141592653589793238462643;
            pi = h * aux;
            printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }
    MPI_Finalize();
}