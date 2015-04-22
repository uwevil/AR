#ifndef ____projet__
#define ____projet__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <CommonCrypto/CommonDigest.h>
#include <pthread.h>
#include <mpi.h>

#define TAG_NOEUD   0
#define TAG_DATA    1
#define TAG_SEARCH  2
#define TAG_OK      3
#define TAG_ADD     4
#define TAG_DELETE  5
#define TAG_UPDATE  6

#define min(a,b) ((a <= b) ? a : b)
#define max(a,b) ((a >= b) ? a : b)

typedef struct poin{
    int x;
    int y;
} point;

struct liste_voisins{
    int id;
    point min, max;
    int pos; // 0haut, 1droit, 2bas, 3gauche
    struct liste_voisins *next;
};

typedef struct liste_voisins voisins;

struct donnee{
    int val;
    point p;
    struct donnee *next;
    struct donnee *prev;
} ;

struct local{
    point p;
    point min, max;
    int nb_vois;
    struct donnee *d;
    voisins *v;
};





#endif /* defined(____projet__) */
