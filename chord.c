#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//tags
#define TAG_INIT 0	//initialise les champs
#define TAG_LOOKUP 1	//parcours
#define TAG_LASTCHANCE 2	//quand lookup est fini
#define TAG_RESULT 3	//quand lastchance est fini
#define TAG_TERMINAISON 4	//quand result est fini

#define NB_SITES 4
#define NB_CLES 3

#define M 	16

int rang;	// rang MPI
MPI_Status status; //status mpi
int buf[2];	//buffer d'envoies
int chord;	// identifiant chord
int test;

//tableau de cles
int keys[NB_CLES];

struct finger {	//plus simple pour le stockage
	int rank;	// rang MPI pour les fingers pour envoyer des messages
	int chord;	// identifiant chord pour les fingers
};
//tableau de fingers
struct finger fingers[M];

//donne les chords aux differents sites
void simulateur(void) {
	int i, j, k, tmp;
	int sites[NB_SITES];
	
	//donne un chord distinct a chaque rank
	struct finger fingers_tmp[M] = { 0 };

	for (i = 1; i <= NB_SITES; i++)
	{
		//permet d'eviter les doublons dans sites
		sites[i] = rand() % (1 << M);
		for (j = 1; j < i; j++) {	//ne teste que ceux deja assignes
			if (sites[i] == sites[j]) {
				sites[i] = (sites[i] + 1) % (1 << M);
				j = 0;
			}
		}
	}

	//creer fingers pour chaque site
	for (i = 1; i < NB_SITES; i++)
	{
		for (j = 0; j < M; j++) {
			tmp = (i + (1 << j)) % (1 << M);
			fingers_tmp[j].chord = (1 << M);

			//on prends le plus petit chord correspondant
			for (k = 1; k < NB_SITES; k++) {
				if (sites[k] >= tmp && sites[k] < fingers_tmp[j].chord) {
					fingers_tmp[j].chord = sites[k];
					fingers_tmp[j].rank = k;
				}
			}
		}
		//envois aux autres processus
		MPI_Send(&sites[i], 1, MPI_INT, i, TAG_INIT, MPI_COMM_WORLD);
		MPI_Send(&fingers_tmp[0], sizeof(fingers_tmp), MPI_INT, i, TAG_INIT, MPI_COMM_WORLD);
	}
}

//init les autres que 0
void init(void) {

	MPI_Recv(&chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&fingers[0], sizeof(fingers), MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
	keys[0] = chord;
	keys[1] = chord - 2;	//cles aleatoires
	keys[2] = chord - 4;	//cles aleatoires
	//printf("le site de rang %d et de chord %d a fini de s'initialiser\n", rang, chord);
}

//cherche qui s'occupe de la cle
struct finger *findnext(int key) {
	struct finger *tmp;
	for (int i = M-1; i >= 0; i--) {
		tmp = &fingers[i];
		if (tmp->chord < key && key <= chord) {
			return fingers[i].chord == (1 << M) ? NULL : &fingers[i];
		}
	}
	return NULL;
}

//fonction pour circuler dans l'anneau
void lookup(int src, int key) {
	buf[0] = src;
	buf[1] = key;
	struct finger *next = findnext(key);
	//printf("le site de rang %d et de chord %d fait un lookup\n", rang, chord);
	if (next == NULL)
		MPI_Send(&buf[0], 2, MPI_INT, fingers[0].rank, TAG_LASTCHANCE, MPI_COMM_WORLD);
	else
		MPI_Send(&buf[0], 2, MPI_INT, next->rank, TAG_LOOKUP, MPI_COMM_WORLD);
}

//fonction qui traite tous les messages recu
void reception(void) {
	MPI_Recv(&buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	if (rang == 0){
			for (int i = 1; i < NB_SITES; i++)
				MPI_Send(&buf[0], 2, MPI_INT, i, TAG_TERMINAISON, MPI_COMM_WORLD);
			MPI_Finalize();
			exit(EXIT_SUCCESS);
			
	}else if (status.MPI_TAG == TAG_LOOKUP)
		lookup(buf[0], buf[1]);

	else if (status.MPI_TAG == TAG_LASTCHANCE) {
		int i;
		int src = buf[0];
		buf[0] = fingers[0].rank;
		for (i = 0; i < NB_CLES; i++) {
			if (buf[1] == keys[i]) {//lastchance reussis
				buf[1] = chord;
				MPI_Send(&buf[0], 2, MPI_INT, src, TAG_RESULT, MPI_COMM_WORLD);
			}
		}

		if (i == NB_CLES) {//lastchance echoue (on a fait le maximum d'iteratioon avant)
			buf[1] = -1;
			MPI_Send(&buf[0], 2, MPI_INT, src, TAG_RESULT, MPI_COMM_WORLD);
		}
	}else if (status.MPI_TAG == TAG_RESULT) {
		if (buf[1] != -1)
			printf("le site ayant la cle est %d\n", buf[1]);
		else
			printf("Pas de site pour la cle\n");
	}else if (status.MPI_TAG == TAG_TERMINAISON) {//sort de la boucle du main
		test = 0;
	}
}

//main copié/collé d'un autre tme avec quelques ajustements
int main(int argc, char* argv[])
{
	srand(time(0));
	int nb_proc, site_x;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

	if (nb_proc != NB_SITES)
	{
		printf("Nombre de processus incorrect !\n");
		MPI_Finalize();
		exit(2);
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);

	test = 1;

	if (rang == 0)
	{
		simulateur();	//lance les autres procs

		site_x = 1 + (rand() % (NB_SITES-1));	//site au quel on envois different de 0
		buf[0] = 0;
		buf[1] = rand() % (1 << M);//cle quelconque
		printf("cle = %d\n", buf[1]);
		MPI_Send(&buf[0], 2, MPI_INT, site_x, TAG_LOOKUP, MPI_COMM_WORLD);
		reception();
	}else{
		init();
		while (test)
			reception();
		printf("le site de rang %d et de chord %d se termine\n", rang, chord);
	}
	MPI_Finalize();
	return EXIT_SUCCESS;
}
