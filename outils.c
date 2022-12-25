#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

void tri(int max, struct node *nodes, int min) {
	int i = min - 1, pivot = nodes[max].key;
	
	if (min < max) {
		for (int j = min; j <= max - 1; j++) {
			if (nodes[j].key < pivot) {
				i++;
				// xor swap entre nodes[i] et nodes[j]
				nodes[i] ^= nodes[j];
    			nodes[j] ^= nodes[i];
    			nodes[i] ^= nodes[j];
			}
		}
		nodes[i+1] ^= nodes[max];
   		nodes[max] ^= nodes[i+1];
    	nodes[i+1] ^= nodes[max];
		tri(i, nodes, min);
		tri(max, nodes, i+2);
	}
}

void afficheN(size_t taille, struct node *no, int testFinger) {
	for (int i = 0; i < taille; i++) {
		printf("|- noeud CHORD : %d, rang MPI %d", no[i].cle, no[i].rang);
		if (testFinger) 
			for (int j = 0; j < W; j++) 
				printf(" finger N°%d : noeud CHORD %d rang MPI %d | ", j, no[i].fingers[j]->cle, no[i].fingers[j]->rang);
		printf("-|\n");
	}
}