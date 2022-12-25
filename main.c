#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "conteneurs.h"
#include "outils.h"
#include "testeur.h"

int main(int argc, char **argv) {
	int maxi, rang, taille;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);
	MPI_Comm_size(MPI_COMM_WORLD, &taille);
	maxi = pow(2, W);

	if (rang != taille - 1) {
		gen_noeud(taille - 1, rang, maxi);
	} else {
		test( maxi , taille - 1);
	}
   	MPI_Finalize();
	return 0;   
}