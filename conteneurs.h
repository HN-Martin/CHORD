#ifndef conteneur
#define conteneur

#define PEERS 5
#define OVER 3
#define QUERY 2
#define PEER 4
#define INIT 1

#define W 6
#define successor(n) n.fingers[0]

struct query {
	int suiv; 							/* le noeud à qui répondre */ 
	int cle; 							/* clé de recherche */
};

struct noeud {
	struct noeud *fingers;
	int cle; 							/* clé CHORD */
	int rang; 							/* l'identifiant physique du noeud */ 
};

#endif