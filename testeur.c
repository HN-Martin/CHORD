#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "testeur.h"

void test(int max, int n) {                                         /* n : taille, max : le max des noeuds */ 
	int id, unique = 0 , start;
	srand(getpid());
	struct noeud nodes[n];
	MPI_Status status;

	int IDvect[n];
	memset(IDvect, 0, n * sizeof(int));
	
	printf("Creation des noeuds\n");
	for (int i = 0; i < n; i++) {                                   /* On génère l'id des noeuds */
		while(!unique) {
			id = rand()%max;
			unique = 1;
			for (int j = 0; j < n; j++) 
				if (IDvect[j] == id) 
					unique = 0;							
		}
		nodes[i].cle = id;
		nodes[i].rang = i;
		unique = 0;
	}

	for (int i = 0; i < n; i++) {                                /* Envoie de l'id du noeud  */
		MPI_Send(&nodes[i], 1, MPI_INT, i, INIT, MPI_COMM_WORLD);
		start = rand()%2;
		MPI_Send(&start, 1, MPI_INT, i, INIT, MPI_COMM_WORLD);
	}
}

void gen_noeud(int l, int n,  int max) {																		/* l: taille , n: rang */
	struct noeud fingers[W], cur, *voisins = (struct noeud *)malloc(l * sizeof(struct noeud));
	int deb, start, next, recu, val, test = 1;
	MPI_Status status;

	voisins[n].rang = n;
	MPI_Recv(&val, 1, MPI_INT, l, INIT, MPI_COMM_WORLD, &status);
	voisins[n].cle = val;
	MPI_Recv(&start, 1, MPI_INT, l, INIT, MPI_COMM_WORLD, &status);
	next = (n + 1) % l;

	if (start) {
		printf("n %d est l'initiateur, il envoie à %d\n", n, next);
		MPI_Send(&n, 1, MPI_INT, next, PEER, MPI_COMM_WORLD);
		MPI_Send(voisins, l * (sizeof(struct noeud)), MPI_CHAR, next, PEER, MPI_COMM_WORLD);
	}

	while (test) {
		MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(voisins, l * (sizeof(struct noeud)), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (status.MPI_TAG == PEER) {
			if (start && recu < n) {
				printf("n %d a reçu un message d'un noeud inférieur: le n° %d \n", n, recu);
			} else if (recu == n) {
				printf("n %d a reçu son propre rang cad %d \n", n, PEERS);
				MPI_Send(&recu, 1, MPI_INT, next, PEERS, MPI_COMM_WORLD);
				MPI_Send(voisins, l * (sizeof(struct noeud)), MPI_CHAR, next, PEERS, MPI_COMM_WORLD);
			} else {
				printf("n %d transmission au suivant %d \n", n, next);
				voisins[n].cle = val;
				voisins[n].rang = n;
				MPI_Send(&recu, 1, MPI_INT, next, PEER, MPI_COMM_WORLD);
				MPI_Send(voisins, l * (sizeof(struct noeud)), MPI_CHAR, next, PEER, MPI_COMM_WORLD);
			}
		} else if (status.MPI_TAG == PEERS) {
			printf("n %d a reçu la valeure PEERS et a donc terminé\n", n);
			if (recu != n) {
				MPI_Send(&recu, 1, MPI_INT, next, PEERS, MPI_COMM_WORLD);
				MPI_Send(voisins, l * (sizeof(struct noeud)), MPI_CHAR, next, PEERS, MPI_COMM_WORLD);
			}
			test = 0;
		} else {
			printf("Erreur\n");
		}
	}
	
	/* On génère les finger pour chaque noeud grace à la liste en allant du max au min pour trouver le premier dont l'index est supérieure à deb étant le finger cherché */
	tri(l - 1, voisins, 0);
	for (int j = 0; j < W; j++) {					
		trouve = 0;
		deb = (val + (int)pow(2, j))% max;
		fingers[j] = voisins[0];              // valeure par défaut au cas où non trouvé
		for (int x = 0; x < l; x++)		
			if (voisins[x].cle >= deb) {
				fingers[j] = voisins[x];
				break;
			}
	}

	cur.rang = n;
	cur.cle = val;
	for (int i = 0; i < W; i++) {
		cur.fingers[i] = &fingers[i];
	}

	if (n == 0) {
		afficheN(&cur, 1, 1);
		printf("n %d noeud %d fingers : \n", cur.rang, cur.cle);
		MPI_Send(&val, 1, MPI_INT, next, INIT, MPI_COMM_WORLD);
	} else if (n == l - 1) {
		MPI_Recv(&deb, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		afficheN(&cur, 1, 1);
		printf("n %d noeud %d fingers : \n", cur.rang, cur.cle);
	} else {
		MPI_Recv(&deb, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		afficheN(&cur, 1, 1);
		printf("n %d noeud %d fingers : \n", cur.rang, cur.cle);
		MPI_Send(&val, 1, MPI_INT, next, INIT, MPI_COMM_WORLD);
	}	
	free(voisins);
}