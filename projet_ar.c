#include "projet_ar.h"

//***************************************************************************************************
// Ce projet est réalisé par Tai Liqun et DOAN Cao Sang 3002808                                     *
//                                                                                                  *
// Les étapes: insertion de noeud, insertion de données et recherche de données marchent très bien. *
// L'étape suppression noeud et recherche après la suppression fonctionnent.                        *
//***************************************************************************************************

//***************************************************************************************************
// Coordinateur se charge de de la réception des requêtes d'insertion d'un noeud et génère les      *
// requêtes d'addition de données dans l'overlay. En plus, il génère aussi les requêtes de recherche*
// des données. A la fin, il va supprimer un noeud où se trouve la donnée demandée et tester avec   *
// cette donnée pour tester le nouveau endroit où la stocke dans l'overlay.                         *
//***************************************************************************************************

void coordinateur(int nb_proc){
    char data[size];
    MPI_Status status;
    int source;
    int i = 0;
    
    memset((void *)data, '\0', sizeof(char)*strlen(data));
    
//recoit la réponse du noeud 1
    MPI_Recv(data, size, MPI_CHAR, 1, TAG_OK, MPI_COMM_WORLD, &status);
    printf("\n....................adding node.................\n");

    for (i = 0; i < nb_proc - 2; i++) {
        memset((void *)data, '\0', sizeof(char)*strlen(data));
//écoute les demandes d'ajout au overlay
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_NOEUD, MPI_COMM_WORLD, &status);
        
        source = status.MPI_SOURCE;
        printf("insertion in overlay of id %d\n", source);
       
//envoie la demande au noeud 1
        MPI_Send(data, size, MPI_CHAR, 1, TAG_NOEUD, MPI_COMM_WORLD);
        printf("ajout de noeud %s ", data);
        
        memset((void *)data, '\0', sizeof(char)*strlen(data));
//recoit la confirmation d'avoir été ajouté à overlay
        MPI_Recv(data, size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD, &status);
    }
    
    struct donnee mem[10];
    int j = 0;
    printf("....................adding data.................\n");

    for (i = 0; i < 10*nb_proc; i++) {
        int x, y;
        x = rand() % 1000; // génére la valeur aléatoire
        y = rand() % 1000; // génére la valeur aléatoire
        
//stocke les 5 premières valeurs et 5 dernières
        if ((i < 5) || (i >= 10*nb_proc - 5)) {
            mem[j].p.x = x;
            mem[j].p.y = y;
            mem[j].val = x + y;
            j++;
        }
        memset((void *)data, '\0', sizeof(char)*strlen(data));

//envoie le requete d'ajout des données sous forme 'x;y;x+y'
        sprintf(data, "%d;%d;%d\n", x, y, x + y);
        MPI_Send(data, size, MPI_CHAR, 1, TAG_DATA, MPI_COMM_WORLD);
        printf("ajout de donnee %s", data);
       
        memset((void *)data, '\0', sizeof(char)*strlen(data));
//recoit la confirmation
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    }
    
    printf("....................seraching data.................\n");
    for (i = 0; i < 14; i++) {
        int x, y;
        
        if (i >= 10) {
// génère les 5 dernières valeurs aléatoires pour la recherche
            x = rand() % 1000;
            y = rand() % 1000;
        }
        else{
// 5 premières et 5 dernières valeurs stockées de l'étape "ajout data"
            x = mem[i].p.x;
            y = mem[i].p.y;
        }
        
        memset((void *)data, '\0', sizeof(char)*strlen(data));
        
//la requete de recherche sous forme "x;y"
        sprintf(data, "%d;%d", x, y);
        MPI_Send(data, size, MPI_CHAR, 1, TAG_SEARCH, MPI_COMM_WORLD);
   
        memset((void *)data, '\0', sizeof(char)*strlen(data));

//recoit la réponse avec le noeud qui contient la donnée
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
        printf("noeud %d repond %s", status.MPI_SOURCE, data);
    }
    
    printf("....................deleting node.................\n");
    
    memset((void *)data, '\0', sizeof(char)*strlen(data));

//supprime le noeud qui contient la donnée dans cette coordonnée "x;y"
    sprintf(data, "%d;%d\n", mem[7].p.x, mem[7].p.y);
    MPI_Send(data, size, MPI_CHAR, 1, TAG_NOEUD_DELETE, MPI_COMM_WORLD);
    printf("deleted %s", data);

    memset((void *)data, '\0', sizeof(char)*strlen(data));

//recoit la confirmation de la suppression
    MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    
    printf("....................re-rearching.................\n");
    
    memset((void *)data, '\0', sizeof(char)*strlen(data));

//recherche à nouveau la donnée ou se trouve dans le noeud supprimé
    sprintf(data, "%d;%d\n", mem[7].p.x, mem[7].p.y);
    MPI_Send(data, size, MPI_CHAR, 1, TAG_SEARCH, MPI_COMM_WORLD);
    printf("recherche %s => ", data);

    memset((void *)data, '\0', sizeof(char)*strlen(data));
    
//recoit la réponse avec la valeur ou se trouve dans le noeud supprimé
    MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    printf("id %d repond %s", status.MPI_SOURCE, data);
    
}

//***************************************************************************************************
// La fonction "est_voisins" qui calcule si 2 espaces sont adjacents                                *
//***************************************************************************************************

int est_voisins(point a, point b, point c, point d){
    if (((a.x == c.x) && (( b.y == c.y) || (a.y == d.y))) ||
        ((b.x == c.x) && (( b.y == d.y) || (a.y == c.y))) ||
        ((a.x == d.x) && (( b.y == d.y) || (a.y == c.y))) ||
        ((b.x == d.x) && (( b.y == c.y) || (a.y == d.y)))) {
        return 1;
    }
    return 0;
}

//***************************************************************************************************
// La fonction qui joue le rôle d'un noeud                                                          *
//***************************************************************************************************

void noeud(int rang){
    struct local l;
    char buf[size], buf_tmp1[1024], buf_tmp2[1024];

    char data[size], *data_tmp;
    MPI_Status status;
    
/**Initialisation**/
    if (rang == 1) {
        
// Le noeud 1 génère ses coordonnées et les envoies au coordinateur
        l.p.x = rand() % 1000;
        l.p.y = rand() % 1000;
        l.min.x = 0;
        l.min.y = 0;
        l.max.x = 1000;
        l.max.y = 1000;
        l.nb_vois = 0;
        l.d = NULL;
        l.v = NULL;
        MPI_Send("", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
    }
    else{
/*
 Les autres noeuds différents que le noeud 1 doivent envoyer leurs coordonnées au coordinateur pour être placés
 dans l'overlay.
 La requête est sous forme:
    x;y;xmin;ymin;xmax;ymax;nb-voisins;id1;xmin1;ymin1;xmax1;ymax1;id2....
*/
        l.p.x = rand() % 1000;
        l.p.y = rand() % 1000;
        l.d = NULL;
        
//génère les coordonnées et envoie la demande au coordinateur pour être placé dans overlay
        sprintf(data, "%d;%d;%d\n", rang, l.p.x, l.p.y);

        MPI_Send(data, size, MPI_CHAR, 0, TAG_NOEUD, MPI_COMM_WORLD);
        
//la réponse est sous forme "x;y;xmin;ymin;xmax;ymax;nombre-voisins;id-voisin1;xmin1;xymin1;xmax1;ymax2;..."
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);

        l.p.x = atoi(strtok(data, ";"));
        l.p.y = atoi(strtok(NULL, ";"));
        l.min.x = atoi(strtok(NULL, ";"));
        l.min.y = atoi(strtok(NULL, ";"));
        l.max.x = atoi(strtok(NULL, ";"));
        l.max.y = atoi(strtok(NULL, ";"));
        l.nb_vois = atoi(strtok(NULL, ";"));
    
        int i;
        voisins *tmp;
        
        printf(" position réelle -%d-%d-%d-\n", rang, l.p.x, l.p.y);

//teste si il y a au moins un voisin
        if (l.nb_vois > 0) {
            l.v = (voisins *)malloc(sizeof(voisins));
            tmp = l.v;
            
            for (i = 0; i < l.nb_vois; i++) {
                sprintf(buf, "%d;%d;%d;%d\n", l.min.x, l.min.y, l.max.x, l.max.y);

                tmp->id = atoi(strtok(NULL, ";"));
                
                (tmp->min).x = atoi(strtok(NULL, ";"));
                (tmp->min).y = atoi(strtok(NULL, ";"));
                (tmp->max).x = atoi(strtok(NULL, ";"));
                (tmp->max).y = atoi(strtok(NULL, ";"));
                
                if (tmp->min.x >= l.max.x) {
                    tmp->pos = 1;
                }else if(tmp->max.x <= l.min.x){
                    tmp->pos = 3;
                }else if(tmp->min.y >= l.max.y){
                    tmp->pos = 0;
                }else{
                    tmp->pos = 2;
                }
                
//si oui, ajout le voisin dans la liste des voisins et notifie le voisin pour qu'il puisse ajouter dans sa propre liste des voisins
//le message est sous forme "xmin;ymin;xmax;ymax" de l'émetteur
                MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_ADD, MPI_COMM_WORLD);
                MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                memset((void *)buf, '\0', sizeof(char)*strlen(buf));

                if (i + 1 < l.nb_vois) {
                    tmp->next = (voisins *)malloc(sizeof(voisins));
                }else{
                    tmp->next = NULL;
                }
                tmp = tmp->next;
            }
        }else{
            l.v = NULL;
        }
        MPI_Send("", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
    }
// Après l'insertion d'un noeud dans l'overlay, le noeud écoute les messages
    
/**ecoute**/
    int ok = 1;
    while (ok) {
        memset((void *)data, '\0', sizeof(char)*strlen(data));

//écoute les messages à l'entrée
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        data[strlen(data)] = '\0';
        int source = status.MPI_SOURCE;
        
        int i, j, q, r, t, x, y, valeur, id_req, x_req, y_req;
        int route;
        voisins *tmp = l.v;
        voisins *prev;
        point k, m, n, o;
        point z1, z2;
        struct donnee *d_tmp, *d_tmp1;

        memset((void *)buf, '\0', sizeof(char)*strlen(buf));
        memset((void *)buf_tmp1, '\0', sizeof(char)*strlen(buf_tmp1));
        memset((void *)buf_tmp2, '\0', sizeof(char)*strlen(buf_tmp2));

// Il y a 10 types de messages:
//***************************************************************************************************
// 0 TAG_NOEUD       : pour ajouter un noeud dans l'overlay                                         *
// 1 TAG_DATA        : pour ajouter la donnée dans le bon endroit                                   *
// 2 TAG_SEARCH      : pour rechercher la valeur d'une donnée dans l'overlay. La valeur reçoit égale*
//                      à -1, c-à-d, il n'existe pas.                                               *
// 3 TAG_OK          : pour confirmer la bonne réception                                            *
// 4 TAG_ADD         : pour demander d'ajouter un voisin dans la liste des voisins                  *
// 5 TAG_DELETE      : pour demander de supprimer un voisins dans la liste des voisins              *
// 6 TAG_UPDATE      : pour demander de mettre à jour la liste des voisins                          *
// 7 TAG_NOEUD_DELETE: pour demander la suppression d'un noeud dans l'overlay                       *
// 8 TAG_NOEUD_UPDATE: pour rendre l'espace du noeud supprimé au nouveau noeud                      *
// 9 TAG_NOEUD_DATA  : pour rendre les données du noeud supprimé au nouveau noeud                   *
//***************************************************************************************************

        switch (status.MPI_TAG) {
            case TAG_NOEUD:
// Confirme la bonne réception au l'émetteur sauf le coordimateur
                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }
// La requête est sous forme "id;x;y"
                id_req = atoi(strtok(data, ";"));
                x_req = atoi(strtok(NULL, ";"));
                y_req = atoi(strtok(NULL, ";"));

// Route le message
//***************************************************************************************************
// 0 : vers le haut                                                                                 *
// 1 : vers le droit                                                                                *
// 2 : vers le bas                                                                                  *
// 3 : vers la gauche                                                                               *
//***************************************************************************************************

                if ((x_req < l.min.x) || (x_req > l.max.x) ||
                    (y_req < l.min.y) || (y_req > l.max.y)) {
// Si ce noeud n'est pas destinataire final, il tranfert le message
                    if (x_req > l.max.x) {
                        route = 1;
                    }else if (x_req < l.min.x){
                        route = 3;
                    }else if( y_req > l.max.y){
                        route = 0;
                    }else{
                        route = 2;
                    }
// envoie le message selon la route choisie
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d;%d\n", id_req, x_req, y_req);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }

                }else{
// si ce noeud est bien le destinataire du message
                    k = l.min;
                    o = l.max;

// calcul la division d'espace entre le nouveau noeud et l'ancien noeud
                    if ((l.max.x - l.min.x) >= (l.max.y - l.min.y)){
                        m.x = (l.min.x + l.max.x) / 2;
                        m.y = l.min.y;
                        n.x = (l.min.x + l.max.x) / 2;
                        n.y = l.max.y;
                        
                        if (l.p.x <= n.x) {
                            l.max.x = n.x;
                            l.max.y = n.y;
                            
                            z1.x = m.x;
                            z1.y = m.y;
                            
                            z2.x = o.x;
                            z2.y = o.y;
                        }else{
                            l.min.x = m.x;
                            l.min.y = m.y;
                            
                            z1.x = k.x;
                            z1.y = k.y;
                            
                            z2.x = n.x;
                            z2.y = n.y;
                        }
                    }else{
                        n.y = (l.min.y + l.max.y) / 2;
                        n.x = l.max.x;
                        m.y = (l.min.y + l.max.y) / 2;
                        m.x = l.min.x;
                        
                        if (l.p.y <= n.y) {
                            l.max.x = n.x;
                            l.max.y = n.y;
                            
                            z1.x = m.x;
                            z1.y = m.y;
                            
                            z2.x = o.x;
                            z2.y = o.y;
                        }else{
                            l.min.x = m.x;
                            l.min.y = m.y;
                            
                            z1.x = k.x;
                            z1.y = k.y;
                            
                            z2.x = n.x;
                            z2.y = n.y;
                        }
                    }

//si les coordonnées de nouveau noeud ne correspond pas au nouveau espace, les génère à nouveau
                    while ((x_req < z1.x) || (x_req > z2.x)) {
                        x_req = rand() % 1000;
                    }
                    
                    while ( (y_req < z1.y) || (y_req > z2.y)) {
                        y_req = rand() % 1000;
                    }
                    
                    t = 0;
                    tmp = l.v;
                    prev = l.v;

                    for (i = 0; i < l.nb_vois && tmp != NULL; i++) {
                        if (est_voisins(z1, z2, tmp->min, tmp->max)){
// teste si les voisins de l'ancien noeud sont bien les voisins de nouveau noeud, si oui, prépare pour les
//      envoyer au nouveau noeud
                            t++;
                            sprintf(buf_tmp2, ";%d;%d;%d;%d;%d", tmp->id, (tmp->min).x, (tmp->min).y, (tmp->max).x, (tmp->max).y);
                            strcat(buf_tmp1, buf_tmp2);
                        }
                        if (est_voisins(l.min, l.max, tmp->min, tmp->max)) {
// test si les voisins de l'ancien noeud sont bien voisins de ce noeud avec le nouveau espace, si oui, les notifie le changement d'espace.
                            sprintf(buf, "%d;%d;%d;%d\n", l.min.x, l.min.y, l.max.x, l.max.y);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_UPDATE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            prev = tmp;
                            tmp = tmp->next;
                        }else{
// si les voisins ne sont plus
                            MPI_Send("", size, MPI_CHAR, tmp->id, TAG_DELETE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);

                            if (tmp == l.v) {
                                l.v = tmp->next;
                                free(tmp);
                                tmp = l.v;
                                prev = l.v;
                                l.nb_vois--;
                                i--;
                            }else{
                                prev->next = tmp->next;
                                free(tmp);
                                l.nb_vois--;
                                i--;
                                tmp = prev->next;
                            }
                        }
                    }
                    sprintf(buf_tmp2, ";%d;%d;%d;%d;%d", rang, l.min.x, (l.min).y, (l.max).x, (l.max).y);
                    strcat(buf_tmp1, buf_tmp2);
                    sprintf(buf, "%d;%d;%d;%d;%d;%d;%d", x_req, y_req, z1.x, z1.y, z2.x, z2.y, t+1);
                    strcat(buf, buf_tmp1);
                    buf[strlen(buf)] = '\0';
// répond au demandeur ses coordonnées et son propre espace à gérer avec les voisins
                    MPI_Send(buf, size, MPI_CHAR, id_req, TAG_OK, MPI_COMM_WORLD);
                }
                break;
                
            case TAG_DELETE:
// supprime le voisin( émetteur) dans la liste des voisins du récepteur
                for (i = 0; i < l.nb_vois && tmp != NULL; i++, tmp = tmp->next) {
                    if (tmp->id == source) {
                        if (tmp == l.v) {
                            l.v = tmp->next;
                            free(tmp);
                            tmp = l.v;
                            MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);

                            break;
                        }else{
                            prev->next = tmp->next;
                            free(tmp);
                            tmp = prev->next;
                            MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);

                            break;
                        }
                    }
                    prev = tmp;
                }
                l.nb_vois--;
                break;
                
            case TAG_ADD:
// ajoute un voisin( émetteur) s'il n'existe pas dans la liste des voisins du récepteur
                if (l.v == NULL) {
                    l.v = (voisins *)malloc(sizeof(voisins));
                    l.v->id = source;
                    (l.v->min).x = atoi(strtok(data, ";"));
                    (l.v->min).y = atoi(strtok(NULL, ";"));
                    (l.v->max).x = atoi(strtok(NULL, ";"));
                    (l.v->max).y = atoi(strtok(NULL, ";"));
                    l.v->next = NULL;
                    tmp = l.v;
                }else{
                    prev = l.v;
                    tmp = (voisins *)malloc(sizeof(voisins));
                    tmp->id = source;
                    // car le mess sous forme xmin;ymin;xmax;ymax
                    (tmp->min).x = atoi(strtok(data, ";"));
                    (tmp->min).y = atoi(strtok(NULL, ";"));
                    (tmp->max).x = atoi(strtok(NULL, ";"));
                    (tmp->max).y = atoi(strtok(NULL, ";"));
                    tmp->next = prev;
                    l.v = tmp;
                }
                
                if (tmp->max.x == l.min.x) {
                    tmp->pos = 3;
                }else if (tmp->min.x == l.max.x){
                    tmp->pos = 1;
                }else if (tmp->min.y == l.max.y){
                    tmp->pos = 0;
                }else{
                    tmp->pos = 2;
                }
                
                l.nb_vois++;
                
                MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);

                break;
                
            case TAG_UPDATE:
// met à jour un voisins( émetteur) dans la liste des voisins du récepteur
                for (i = 0; i < l.nb_vois && tmp != NULL; i++, tmp = tmp->next) {
                    if (source == tmp->id) {

                        (tmp->min).x = atoi(strtok(data, ";"));
                        (tmp->min).y = atoi(strtok(NULL, ";"));
                        (tmp->max).x = atoi(strtok(NULL, ";"));
                        (tmp->max).y = atoi(strtok(NULL, ";"));
                        
                        if (tmp->max.x == l.min.x) {
                            tmp->pos = 3;
                        }else if (tmp->min.x == l.max.x){
                            tmp->pos = 1;
                        }else if (tmp->min.y == l.max.y){
                            tmp->pos = 0;
                        }else{
                            tmp->pos = 2;
                        }
                        MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);

                        break;
                    }else if(tmp->next == NULL){
                        tmp->next = (voisins *)malloc(sizeof(voisins));
                        tmp = tmp->next;
                        tmp->id = source;
                        
                        (tmp->min).x = atoi(strtok(data, ";"));
                        (tmp->min).y = atoi(strtok(NULL, ";"));
                        (tmp->max).x = atoi(strtok(NULL, ";"));
                        (tmp->max).y = atoi(strtok(NULL, ";"));
                        
                        if (tmp->max.x == l.min.x) {
                            tmp->pos = 3;
                        }else if (tmp->min.x == l.max.x){
                            tmp->pos = 1;
                        }else if (tmp->min.y == l.max.y){
                            tmp->pos = 0;
                        }else{
                            tmp->pos = 2;
                        }

                        l.nb_vois++;
                        MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);

                        break;
                    }
                }
               
                break;
                
            case TAG_DATA:
// ajoute d'une donnée dans le bon endroit
                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }
                
// data_message contient x;y;valeur
                
                x = atoi(strtok(data, ";"));
                y = atoi(strtok(NULL, ";"));
                valeur = atoi(strtok(NULL, ";"));
                
                if ((x < l.min.x) || (x > l.max.x) || (y > l.max.y) || (y < l.min.y)) {
                    if (x > l.max.x) {
                        route = 1;
                    }else if (x < l.min.x){
                        route = 3;
                    }else if( y > l.max.y){
                        route = 0;
                    }else{
                        route = 2;
                    }
// Si ce n'est pas le bon endroit, transfert le message
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d;%d\n", x, y, valeur);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_DATA, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }
                }else{
// si oui, ajoute dans la liste des données gérée par le récepteur
                    if (l.d == NULL) {
                        l.d = (struct donnee *)malloc(sizeof(struct donnee));
                        l.d->val = valeur;
                        (l.d->p).x = x;
                        (l.d->p).y = y;
                        l.d->next = NULL;
                        l.d->prev = NULL;
                    }else{
                        d_tmp = l.d;
                        
                        do{
                            if (((d_tmp->p).x == x) && ((d_tmp->p).y == y)){
                                d_tmp->val = valeur;
                                break;
                            }
                            if (d_tmp->next == NULL) {
                                break;
                            }
                            d_tmp = d_tmp->next;
                        }while(d_tmp != NULL);
                        
                        if (((d_tmp->p).x != x) || ((d_tmp->p).y != y)) {
                            d_tmp = l.d;
                            d_tmp->prev = (struct donnee *)malloc(sizeof(struct donnee));
                            d_tmp->prev->next = d_tmp;
                            d_tmp->prev->prev = NULL;
                            l.d = d_tmp->prev;
                            l.d->val = valeur;
                            (l.d->p).x = x;
                            (l.d->p).y = y;
                        }
                    }
                    MPI_Send("", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
                }
                
                break;
            
            case TAG_SEARCH:
// recherche d'une valeur avec ses coordonnées précises
                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }

                x = atoi(strtok(data, ";"));
                y = atoi(strtok(NULL, ";"));
                valeur = -1;

                if ((x < l.min.x) || (x > l.max.x) || (y > l.max.y) || (y < l.min.y)) {
                    if (x > l.max.x) {
                        route = 1;
                    }else if (x < l.min.x){
                        route = 3;
                    }else if( y > l.max.y){
                        route = 0;
                    }else{
                        route = 2;
                    }
// si les coordonnées dans le message n'appartiennent pas à cette noeud, le tranfert
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d\n", x, y);
                            buf[strlen(buf)] = '\0';

                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_SEARCH, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }
                }else{
// si oui, répond au coordinateur
                    d_tmp = l.d;
                    while (d_tmp != NULL) {
                        if (x == (d_tmp->p).x && y == (d_tmp->p).y) {
                            valeur = (d_tmp)->val;
                            break;
                        }
                        d_tmp = d_tmp->next;
                    }
                    sprintf(buf, "%d;%d;%d\n", x, y, valeur);
                    MPI_Send(buf, size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);

                }
                
                break;
            
            case TAG_NOEUD_DELETE:
// supprime un noeud dans l'overlay
                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }
                q = -1;
                x = atoi(strtok(data, ";"));
                y = atoi(strtok(NULL, ";"));
                
                if ((x < l.min.x) || (x > l.max.x) || (y > l.max.y) || (y < l.min.y)) {
                    if (x > l.max.x) {
                        route = 1;
                    }else if (x < l.min.x){
                        route = 3;
                    }else if( y > l.max.y){
                        route = 0;
                    }else{
                        route = 2;
                    }
// si le coordonnée dans le message n'appartiennent pas à ce noeud, le tranfert
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d\n", x, y);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_DELETE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }
                }else{
// si oui
                    if (l.d == NULL) {
// s'il ne contient aucune donnée
                        printf("Clé ( %d, %d) n'existe pas!\n", x, y);
                    }else{
// si non
                        struct donnee *d_tmp = l.d;
                        
                        do{
                            if (((d_tmp->p).x == x) && ((d_tmp->p).y == y)){
                                break;
                            }
                            if (d_tmp->next == NULL) {
                                break;
                            }
                            d_tmp = d_tmp->next;
                        }while(d_tmp != NULL);
// s'il n'existe pas une donnée à cette coordonnée, affiche le message et répond au coordinateur
                        if (((d_tmp->p).x != x) || ((d_tmp->p).y != y)) {
                            printf("Clé ( %d, %d) n'existe pas!\n", x, y);
                        }else{
// sinon
                            tmp = l.v;
// recherche parmi les voisins, s'il existe un voisin qui a même largeur ou hauteur
                            for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                                if (((l.min.x == (tmp->min).x) && (l.max.x == (tmp->max).x)) ||
                                    ((l.min.y == (tmp->min).y) && (l.max.y == (tmp->max).y))) {
// si oui
// message type id;1;pos;xmin;ymin;xmax;ymax;nb_voisin;id1;xmin;ymin;xmax;ymax;id2...
                                    sprintf(buf, "%d;%d;%d;%d;%d;%d;%d;%d", rang, 1, tmp->pos, l.min.x, l.min.y, l.max.x, l.max.y, l.nb_vois - 1);
                                    
                                    prev = l.v;
                                    for (t = 0; t < l.nb_vois; t++) {
                                        if ((tmp->min.x != prev->min.x) ||
                                            (tmp->min.y != prev->min.y) ||
                                            (tmp->max.x != prev->max.x) ||
                                            (tmp->max.y != prev->max.y)) {
                                            
                                            sprintf(buf_tmp1, ";%d;%d;%d;%d;%d", prev->id, (prev->min).x, (prev->min).y, (prev->max).x, (prev->max).y);
                                            strcat(buf, buf_tmp1);
                                        }
                                        
                                        prev = prev->next;
                                    }
                                    //**********************
                                    buf[strlen(buf)] = '\0';
// envoie sa propre coordonnée avec l'espace géré et la liste des voisins vers le voisin correspondant
                                    MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_UPDATE, MPI_COMM_WORLD);
                                    MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                    //**********************
                                    d_tmp = l.d;
                                    
                                    q = tmp->id;
                                    t = 0;
// envoie des données au voisins correspondant
                                    while (1) {
                                        if (d_tmp == NULL) {
                                            //********************
                                            memset((void *)buf, '\0', sizeof(char)*strlen(buf));

                                            MPI_Send("OK", size, MPI_CHAR, tmp->id, TAG_NOEUD_DATA, MPI_COMM_WORLD);
                                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                            //*********************
                                            break;
                                        }else{
                                            //**************************
                                            memset((void *)buf, '\0', sizeof(char)*strlen(buf));
                                            sprintf(buf, "%d;%d;%d", (d_tmp->p).x, (d_tmp->p).y, d_tmp->val);
                                            buf[strlen(buf)] = '\0';

                                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_DATA, MPI_COMM_WORLD);
                                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);

                                            //**************************
                                            
                                            t++;
                                            d_tmp = d_tmp->next;
                                        }
                                    }
                                    //**********************
                                    //**********************
                                    break;
                                }
                                if (tmp->next == NULL) {
                                    break;
                                }
                            }
// s'il ne trouve pas le voisin qui a même largeur ou hauteur
                            if ((( l.min.x != tmp->min.x) || (l.max.x != tmp->max.x)) &&
                                (( l.min.y != tmp->min.y) || (l.max.y != tmp->max.y))) {
                                
                                // parcourir les 4 positions
                                
                                tmp = l.v;
                                i = tmp->pos;

// il décide de choisir la position du premier voisin dans la liste des voisins et cherche tous les voisins qui ont cette position
                                while (1) {
                                    if (tmp->pos == i) {
                                        sprintf(buf, "%d;%d;%d;%d;%d;%d;%d;%d", rang, 1, tmp->pos, l.min.x, l.min.y, l.max.x, l.max.y, l.nb_vois - 1);
                                        
                                        prev = l.v;
                                        for (t = 0; t < l.nb_vois; t++) {
                                            if ((tmp->min.x != prev->min.x) ||
                                                (tmp->min.y != prev->min.y) ||
                                                (tmp->max.x != prev->max.x) ||
                                                (tmp->max.y != prev->max.y)) {
                                                
                                                sprintf(buf_tmp1, ";%d;%d;%d;%d;%d", prev->id, (prev->min).x, (prev->min).y, (prev->max).x, (prev->max).y);
                                                strcat(buf, buf_tmp1);
                                            }
                                            
                                            prev = prev->next;
                                        }
                                        
                                        //**********************
                                        buf[strlen(buf)] = '\0';
                                        
                                        MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_UPDATE, MPI_COMM_WORLD);
                                        MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                        
                                        //**********************
                                        
                                        d_tmp = l.d;
                                        t = 0;
                                        while (1) {
                                            if (d_tmp == NULL) {
                                                //********************
                                                memset((void *)buf, '\0', sizeof(char)*strlen(buf));
                                                
                                                MPI_Send("OK", size, MPI_CHAR, tmp->id, TAG_NOEUD_DATA, MPI_COMM_WORLD);
                                                MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                                //*********************
                                                break;
                                            }else{
                                                //**************************
                                                memset((void *)buf, '\0', sizeof(char)*strlen(buf));
                                                sprintf(buf, "%d;%d;%d", (d_tmp->p).x, (d_tmp->p).y, d_tmp->val);
                                                buf[strlen(buf)] = '\0';
                                                
                                                MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_DATA, MPI_COMM_WORLD);
                                                MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                                
                                                //**************************
                                                t++;
                                                d_tmp = d_tmp->next;
                                            }
                                        }
                                    }
                                    
                                    if (tmp->next == NULL) {
                                        break;
                                    }else{
                                        tmp = tmp->next;
                                    }
                                }
                            }else{
                                tmp = l.v;
                                i = tmp->pos;
                                if (q == -1) {
                                    while (1) {
                                        if (tmp->pos != i) {
                                            sprintf(buf, "%d;%d", rang, 0);
                                            buf[strlen(buf)] = '\0';
                                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_UPDATE, MPI_COMM_WORLD);
                                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                            
                                        }
                                        if (tmp->next == NULL) {
                                            break;
                                        }
                                        tmp = tmp->next;
                                    }
                                }else{
                                    while (1) {
                                        if (tmp->id != q) {
                                            sprintf(buf, "%d;%d", rang, 0);
                                            buf[strlen(buf)] = '\0';
                                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_UPDATE, MPI_COMM_WORLD);
                                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);

                                        }
                                        if (tmp->next == NULL) {
                                            break;
                                        }
                                        tmp = tmp->next;
                                    }
                                }
                            }
                        }
                    }
                    
                    tmp = l.v;
                    while (tmp) {
                        prev = tmp->next;
                        free(tmp);
                        tmp = prev;
                    }
                    
                    d_tmp = l.d;
                    while (d_tmp) {
                        d_tmp1 = d_tmp->next;
                        free(d_tmp);
                        d_tmp = d_tmp1;
                        l.nb_vois = 0;
                    }
                    ok = 0;
                    MPI_Send("", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
                }
                break;
                
            case TAG_NOEUD_UPDATE:
// met à jour en ajoutant l'espace libre du noeud supprimé
                data_tmp = strdup(data);
                printf("id %d recoit %s\n", rang, data_tmp);
                
                id_req = atoi(strtok(data_tmp, ";"));
                x_req = atoi(strtok(NULL, ";"));
                
                if(x_req == 0){
// s'il ne peut prendre cette espace, il supprime ce noeud dans sa propre liste de voisins
                    prev = tmp = l.v;
  
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++) {
                        if (tmp->id == id_req) {
                            prev->next = tmp->next;
                            free(tmp);
                            tmp = prev->next;
                            l.nb_vois--;
                            i--;

                            break;
                        }
                        prev = tmp;
                        tmp = tmp->next;
                    }
                }else{
// s'il a le droit d'ajouter un morceau de cette espace libre ou cette espace entier, il traite les cas
                    //message type id;1;pos;xmin;ymin;xmax;ymax;nb_voisin;id1;xmin;ymin;xmax;ymax;id2...
                    y_req = atoi(strtok(NULL, ";"));
                    m.x = atoi(strtok(NULL, ";"));
                    m.y = atoi(strtok(NULL, ";"));
                    n.x = atoi(strtok(NULL, ";"));
                    n.y = atoi(strtok(NULL, ";"));
// calcule l'espace qu'il peut ajouter
                    printf("avant la modif %d %d %d %d %d\n", rang, l.min.x, l.min.y, l.max.x, l.max.y);
                    if (y_req == 0) {
                        l.min.y = l.min.y - (n.y - m.y);
                    }else if (y_req == 1){
                        l.min.x = l.min.x - (n.x - m.x);
                    }else if (y_req == 2){
                        l.max.y = l.max.y + (n.y - m.y);
                    }else{
                        l.max.x = l.max.x + (n.x - m.x);
                    }
                    printf("apres la modif %d %d %d %d %d\n", rang, l.min.x, l.min.y, l.max.x, l.max.y);

//nombre de voisins reçu
                    q = atoi(strtok(NULL, ";"));
                    
                    i = 0;
                    for (i = 0; i < q; i++) {
                        y_req = atoi(strtok(NULL, ";"));
                        m.x = atoi(strtok(NULL, ";"));
                        m.y = atoi(strtok(NULL, ";"));
                        n.x = atoi(strtok(NULL, ";"));
                        n.y = atoi(strtok(NULL, ";"));
                        
                        if (est_voisins(l.min, l.max, m, n)){
// si ce nouveau espace est adjacent avec les espaces reçus, le noeud met à jour sa liste de voisins
                            prev = l.v;
                            for (j = 0; j < l.nb_vois && prev != NULL; j++, prev = prev->next) {
                                if (prev->id == y_req) {
                                    break;
                                }
                            }

                            if (prev == NULL) {
                                l.nb_vois++;
                                
                                tmp = (voisins *)malloc(sizeof(voisins));
                                tmp->id = y_req;
                                // car le mess sous forme xmin;ymin;xmax;ymax
                                (tmp->min).x = m.x;
                                (tmp->min).y = m.y;
                                (tmp->max).x = n.x;
                                (tmp->max).y = n.y;
                                
                                prev = l.v;
                                tmp->next = prev;
                                l.v = tmp;
                                if (tmp->max.x == l.min.x) {
                                    tmp->pos = 3;
                                }else if (tmp->min.x == l.max.x){
                                    tmp->pos = 1;
                                }else if (tmp->min.y == l.max.y){
                                    tmp->pos = 0;
                                }else{
                                    tmp->pos = 2;
                                }
                            }
                        }
                    }

                    tmp = l.v;
// notifie les voisins le changement d'espace de ce noeud après l'addition de l'espace libre
                    for (i = 0; i < l.nb_vois; i++) {
                        if (tmp->id != id_req) {
                            sprintf(buf, "%d;%d;%d;%d", (l.min).x, (l.min).y, (l.max).x, (l.max).y);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_UPDATE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            tmp = tmp->next;
                        }
                       
                    }
                    
                    //**********************************************************
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                    //**********************************************************

// reçoit les données de l'ancien noeud
                    while (1) {
                        memset((void *)buf, '\0', sizeof(char)*strlen(buf));
                        MPI_Recv(buf, size, MPI_CHAR, source, TAG_NOEUD_DATA, MPI_COMM_WORLD, &status);

                        if (strlen(buf) <= 5) {
                            break;
                        }
                        
                        MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                        
                        x = atoi(strtok(buf, ";"));
                        y = atoi(strtok(NULL, ";"));
                        valeur = atoi(strtok(NULL, ";"));
                        if ((x >= l.min.x) && (x <= l.max.x) && (y <= l.max.y) && (y >= l.min.y)) {
                            
                            printf("update valeur(s) = %d %d %d\n",x, y, valeur);
                            if (l.d == NULL) {
                                l.d = (struct donnee *)malloc(sizeof(struct donnee));
                                l.d->val = valeur;
                                (l.d->p).x = x;
                                (l.d->p).y = y;
                                l.d->next = NULL;
                                l.d->prev = NULL;
                            }else{
                                d_tmp = l.d;
                                d_tmp->prev = (struct donnee *)malloc(sizeof(struct donnee));
                                d_tmp->prev->next = d_tmp;
                                d_tmp->prev->prev = NULL;
                                l.d = d_tmp->prev;
                                l.d->val = valeur;
                                (l.d->p).x = x;
                                (l.d->p).y = y;
                            }
                        }
                    }
                }
                MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);

                break;
                
         //   default:
           //     break;
        }
        
    }
}

int main(int argc, char ** argv){
    int my_rank;
    int nb_proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    srand (my_rank + time(NULL));

    if (my_rank == 0) {
        coordinateur(nb_proc);
    }else{
        noeud(my_rank);
    }
    
    MPI_Finalize();
    return 0;
}










