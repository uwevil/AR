#include "projet_ar.h"


void coordinateur(int nb_proc){
    char data[size];
    MPI_Status status;
    int source;
    int i = 0;
    
    memset((void *)data, '\0', sizeof(char)*strlen(data));
    
    MPI_Recv(data, size, MPI_CHAR, 1, TAG_OK, MPI_COMM_WORLD, &status);
    printf("\n....................adding node.................\n");

    for (i = 0; i < nb_proc - 2; i++) {
        memset((void *)data, '\0', sizeof(char)*strlen(data));
        
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_NOEUD, MPI_COMM_WORLD, &status);
        
        source = status.MPI_SOURCE;
        printf("insertion in overlay of id %d\n", source);
       
        MPI_Send(data, size, MPI_CHAR, 1, TAG_NOEUD, MPI_COMM_WORLD);
        printf("ajout de noeud %s ", data);
        
        memset((void *)data, '\0', sizeof(char)*strlen(data));

        MPI_Recv(data, size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD, &status);
    }
    
    struct donnee mem[10];
    int j = 0;
    printf("....................adding data.................\n");

    for (i = 0; i < 10*nb_proc; i++) {
        int x, y;
        x = rand() % 1000;
        y = rand() % 1000;
        
        if ((i < 5) || (i >= 10*nb_proc - 5)) {
            mem[j].p.x = x;
            mem[j].p.y = y;
            mem[j].val = x + y;
            j++;
        }
        memset((void *)data, '\0', sizeof(char)*strlen(data));

        sprintf(data, "%d;%d;%d\n", x, y, x + y);
        MPI_Send(data, size, MPI_CHAR, 1, TAG_DATA, MPI_COMM_WORLD);
        printf("ajout de donnee %s", data);
       
        memset((void *)data, '\0', sizeof(char)*strlen(data));

        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    }
    
    printf("....................seraching data.................\n");
    for (i = 0; i < 14; i++) {
        int x, y;
        
        if (i >= 10) {
            x = rand() % 1000;
            y = rand() % 1000;
        }
        else{
            x = mem[i].p.x;
            y = mem[i].p.y;
        }
        
        memset((void *)data, '\0', sizeof(char)*strlen(data));
        
        sprintf(data, "%d;%d", x, y);
        MPI_Send(data, size, MPI_CHAR, 1, TAG_SEARCH, MPI_COMM_WORLD);
    //    printf("recherche %s => reponse from ", data);
   
        memset((void *)data, '\0', sizeof(char)*strlen(data));

        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
        printf("%d = %s", status.MPI_SOURCE, data);
    }
    
    printf("....................deleting node.................\n");
    
    memset((void *)data, '\0', sizeof(char)*strlen(data));

    sprintf(data, "%d;%d\n", mem[7].p.x, mem[7].p.y);
    MPI_Send(data, size, MPI_CHAR, 1, TAG_NOEUD_DELETE, MPI_COMM_WORLD);
    printf("deleted %s", data);

    memset((void *)data, '\0', sizeof(char)*strlen(data));

    MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    
    printf("....................re-rearching.................\n");
    
    memset((void *)data, '\0', sizeof(char)*strlen(data));

    sprintf(data, "%d;%d\n", mem[7].p.x, mem[7].p.y);
    MPI_Send(data, size, MPI_CHAR, 1, TAG_SEARCH, MPI_COMM_WORLD);
    printf("recherche %s => ", data);

    memset((void *)data, '\0', sizeof(char)*strlen(data));

    MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    printf("id %d repond %s", status.MPI_SOURCE, data);
    
}

int est_voisins(point a, point b, point c, point d){
   // printf("est voisins: (%d %d, %d %d)(%d %d, %d %d) => ",a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);
   
    if (((a.x == c.x) && (( b.y == c.y) || (a.y == d.y))) ||
        ((b.x == c.x) && (( b.y == d.y) || (a.y == c.y))) ||
        ((a.x == d.x) && (( b.y == d.y) || (a.y == c.y))) ||
        ((b.x == d.x) && (( b.y == c.y) || (a.y == d.y)))) {
   //     printf("OK\n");

        return 1;
    }
    
  //  printf("NO\n");
    return 0;
}

void noeud(int rang){
    struct local l;
    char buf[size], buf_tmp1[size], buf_tmp2[size];

    char data[size], data_tmp[size];
    MPI_Status status;
    
    /**Initialisation**/
    if (rang == 1) {
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
         x;y;xmin;ymin;xmax;ymax;nb-voisins;id1;xmin1;ymin1;xmax1;ymax1;id2....
         */
        l.p.x = rand() % 1000;
        l.p.y = rand() % 1000;
        l.d = NULL;
        
        sprintf(data, "%d;%d;%d\n", rang, l.p.x, l.p.y);

        MPI_Send(data, size, MPI_CHAR, 0, TAG_NOEUD, MPI_COMM_WORLD);
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

        if (l.nb_vois > 0) {
            l.v = (voisins *)malloc(sizeof(voisins));
            tmp = l.v;
            
            sprintf(buf, "%d;%d;%d;%d\n", l.min.x, l.min.y, l.max.x, l.max.y);

            for (i = 0; i < l.nb_vois; i++) {
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
                
            //    printf("voisin pos -%d-%d-\n", tmp->id, tmp->pos);
                MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_ADD, MPI_COMM_WORLD);

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

    /**ecoute**/
    while (1) {
        memset((void *)data, '\0', sizeof(char)*strlen(data));

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

        switch (status.MPI_TAG) {
            case TAG_NOEUD:

                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }
                
                id_req = atoi(strtok(data, ";"));
                x_req = atoi(strtok(NULL, ";"));
                y_req = atoi(strtok(NULL, ";"));
                
                if ((x_req < l.min.x) || (x_req > l.max.x) ||
                    (y_req < l.min.y) || (y_req > l.max.y)) {
                    if (x_req > l.max.x) {
                        route = 1;
                    }else if (x_req < l.min.x){
                        route = 3;
                    }else if( y_req > l.max.y){
                        route = 0;
                    }else{
                        route = 2;
                    }
               
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d;%d\n", id_req, x_req, y_req);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }

                }else{
                    k = l.min;
                    o = l.max;
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
                            t++;
                            sprintf(buf_tmp2, ";%d;%d;%d;%d;%d", tmp->id, (tmp->min).x, (tmp->min).y, (tmp->max).x, (tmp->max).y);
                            strcat(buf_tmp1, buf_tmp2);
                        }
                        if (est_voisins(l.min, l.max, tmp->min, tmp->max)) {
                            sprintf(buf, "%d;%d;%d;%d\n", l.min.x, l.min.y, l.max.x, l.max.y);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_UPDATE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            prev = tmp;
                            tmp = tmp->next;
                        }else{
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

                    MPI_Send(buf, size, MPI_CHAR, id_req, TAG_OK, MPI_COMM_WORLD);
                }
                break;
                
            case TAG_DELETE:
                
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
                    
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d;%d\n", x, y, valeur);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_DATA, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }
                }else{
                    
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
                    
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d\n", x, y);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_SEARCH, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }
                }else{
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
                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }

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
                    
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d;%d\n", x, y, valeur);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_DELETE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            break;
                        }
                    }
                }else{
                    
                    if (l.d == NULL) {
                        printf("Clé ( %d, %d) n'existe pas!\n", x, y);
                    }else{
                        
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
                        
                        if (((d_tmp->p).x != x) || ((d_tmp->p).y != y)) {
                            printf("Clé ( %d, %d) n'existe pas!\n", x, y);
                        }else{
                            for (i = 0; i < l.nb_vois && tmp != NULL; i ++, tmp = tmp->next) {
                                if (((l.min.x == (tmp->min).x) && (l.max.x == (tmp->max).x)) ||
                                    ((l.min.y == (tmp->min).y) && (l.max.y == (tmp->max).y))) {
                                    
                                    //message type id;1;pos;xmin;ymin;xmax;ymax;nb_voisin;id1;xmin;ymin;xmax;ymax;id2...;nb_data;x;y;valeur;x;y;valeur...
                                    
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
                                    
                                    d_tmp = l.d;
                                    t = 0;
                                    while (1) {
                                        if (d_tmp == NULL) {
                                            break;
                                        }else{
                                            sprintf(buf_tmp1, ";%d;%d;%d", (d_tmp->p).x, (d_tmp->p).y, d_tmp->val);
                                            strcat(buf_tmp2, buf_tmp1);
                                            t++;
                                            d_tmp = d_tmp->next;
                                        }
                                    }
                                    
                                    sprintf(buf_tmp1, ";%d", t);
                                    strcat(buf, buf_tmp1);
                                    strcat(buf, buf_tmp2);

                                    buf[strlen(buf)] = '\0';

                                    MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_UPDATE, MPI_COMM_WORLD);
                                    MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                                    break;
                                }
                                if (tmp->next == NULL) {
                                    break;
                                }
                            }
                            if ((( l.min.x != tmp->min.x) || (l.max.x != tmp->max.x)) &&
                                (( l.min.y != tmp->min.y) || (l.max.y != tmp->max.y))) {
                                
                                // parcourir les 4 positions
                                j = 0;
                                
                                tmp = l.v;
                                i = tmp->pos;
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
                                        
                                        d_tmp = l.d;
                                        t = 0;
                                        while (1) {
                                            if (d_tmp == NULL) {
                                                break;
                                            }else{
                                                sprintf(buf_tmp1, ";%d;%d;%d", (d_tmp->p).x, (d_tmp->p).y, d_tmp->val);
                                                strcat(buf_tmp2, buf_tmp1);
                                                t++;
                                                d_tmp = d_tmp->next;
                                            }
                                        }
                                        
                                        sprintf(buf_tmp1, ";%d", t);
                                        strcat(buf, buf_tmp1);
                                        strcat(buf, buf_tmp2);
                                        
                                        buf[strlen(buf)] = '\0';
                                        
                                        MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_NOEUD_UPDATE, MPI_COMM_WORLD);
                                        MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);

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
                                while (1) {
                                    if (tmp->pos != i) {
                                        sprintf(buf, "%d;%d", rang, 0);
                                        buf[strlen(buf)] = '\0';
                                        MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_DELETE, MPI_COMM_WORLD);
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
                    
                    MPI_Send("", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
 
                }
                break;
                
            case TAG_NOEUD_UPDATE:
                if (source != 0) {
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }

                strcpy(data_tmp, data);
                printf("id %d recoit %s\n", rang, data_tmp);
                
                id_req = atoi(strtok(data_tmp, ";"));
                x_req = atoi(strtok(NULL, ";"));
                
                if(x_req == 0){
                    prev = tmp = l.v;
  
                    for (i = 0; i < l.nb_vois && tmp != NULL; i ++) {
                        if (tmp->id == id_req) {
                            prev->next = tmp->next;
                            free(tmp);
                            tmp = prev->next;
                            l.nb_vois--;
                            i--;
                            MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                            break;
                        }
                        prev = tmp;
                        tmp = tmp->next;
                    }
                }else{
                    //message type id;1;pos;xmin;ymin;xmax;ymax;nb_voisin;id1;xmin;ymin;xmax;ymax;id2...;nb_data;x;y;valeur;x;y;valeu...
                    y_req = atoi(strtok(NULL, ";"));
                    m.x = atoi(strtok(NULL, ";"));
                    m.y = atoi(strtok(NULL, ";"));
                    n.x = atoi(strtok(NULL, ";"));
                    n.y = atoi(strtok(NULL, ";"));
                    
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

                    q = atoi(strtok(NULL, ";"));
                    
                    i = 0;
                    for (i = 0; i < q; i++) {
                        y_req = atoi(strtok(NULL, ";"));
                        m.x = atoi(strtok(NULL, ";"));
                        m.y = atoi(strtok(NULL, ";"));
                        n.x = atoi(strtok(NULL, ";"));
                        n.y = atoi(strtok(NULL, ";"));
                        
                        if (est_voisins(l.min, l.max, m, n)){
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
                    
                    for (i = 0; i < l.nb_vois; i++) {
                        if (tmp->id != id_req) {
                            sprintf(buf, "%d;%d;%d;%d;%d", rang, (l.min).x, (l.min).y, (l.max).x, (l.max).y);
                            MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_UPDATE, MPI_COMM_WORLD);
                            MPI_Recv(buf, size, MPI_CHAR, tmp->id, TAG_OK, MPI_COMM_WORLD, &status);
                            tmp = tmp->next;
                        }
                       
                    }

                    q = atoi(strtok(NULL, ";"));

                    for (i = 0; i < q; i++){

                        x = atoi(strtok(NULL, ";"));
                        y = atoi(strtok(NULL, ";"));
                        valeur = atoi(strtok(NULL, ";"));

                        printf("%d valeur = %d %d %d\n", q, x, y, valeur);
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
                    MPI_Send("", size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }

                break;
                
            default:
                break;
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










