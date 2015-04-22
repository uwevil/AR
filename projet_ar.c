#include "projet_ar.h"


void coordinateur(int nb_proc){
    char data[size];
    MPI_Status status;
    int source;
    int i = 0;
    
    MPI_Recv(data, size, MPI_CHAR, 1, TAG_OK, MPI_COMM_WORLD, &status);

    for (i = 0; i < nb_proc - 2; i++) {
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_NOEUD, MPI_COMM_WORLD, &status);
        source = status.MPI_SOURCE;
        MPI_Send(data, size, MPI_CHAR, 1, TAG_NOEUD, MPI_COMM_WORLD);
        printf("ajout de noeud %s => ", data);
        MPI_Recv(data, size, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD, &status);
        printf("%s\n", data);
    }
    
    struct donnee mem[10];
    int j = 0;

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
        
        sprintf(data, "%d;%d;%d\n", x, y, x + y);
        MPI_Send(data, size, MPI_CHAR, 1, TAG_DATA, MPI_COMM_WORLD);
        printf("ajout de donnee %s =>", data);
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
        printf("%s\n", data);
    }
    
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
            
        
        sprintf(data, "%d;%d", x, y);
        MPI_Send(data, size, MPI_CHAR, 1, TAG_SEARCH, MPI_COMM_WORLD);
        printf("recherche %s = ", data);
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
        printf("%s\n", data);

    }
    
}

int est_voisins(point a, point b, point c, point d){
    if ((a.x == c.x) || (b.x == d.x) || (a.x == d.x) || (b.x == c.x)) {
        if (((a.y <= c.y) && (d.y <= b.y)) || ((a.y >= c.y) && (b.y <= d.y))){
            return 1;
        }
    }
    if ((a.y == c.y) || (b.y == d.y) || (a.y == d.y) || (b.y == c.y)) {
        if (((a.x <= c.x) && (d.x <= b.x)) || ((a.x >= c.x) && (b.x <= d.x))){
            return 1;
        }
    }
    return 0;
}

void noeud(int rang){
    struct local l;
    char buf[size], buf_tmp1[size], buf_tmp2[size];

    char data[size];
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
    
        if (l.nb_vois > 0) {
            l.v = (voisins *)malloc(sizeof(voisins));
            voisins *tmp = l.v;
            
            sprintf(buf, "%d;%d;%d;%d\n", l.min.x, l.min.y, l.max.x, l.max.y);

            for (i = 0; i < l.nb_vois; i++) {
                tmp->id = atoi(strtok(NULL, ";"));
                
                MPI_Send(buf, size, MPI_CHAR, tmp->id, TAG_ADD, MPI_COMM_WORLD);
                
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
        MPI_Send("ajout -> OK", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
    }

    /**ecoute**/
    while (1) {
        MPI_Recv(data, size, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;
        
        int i, t, x, y, valeur, id_req, x_req, y_req;
        int route;
        voisins *tmp = l.v;
        voisins *prev;
        point k, m, n, o;
        point z1, z2;

        switch (status.MPI_TAG) {
            case TAG_NOEUD:

                id_req = atoi(strtok(data, ";"));
                x_req = atoi(strtok(NULL, ";"));
                y_req = atoi(strtok(NULL, ";"));
                
                if ((x_req < l.min.x) || (x_req > l.max.x) || (y_req < l.min.y) || (y_req > l.max.y)) {
                  //  if (x_req >= y_req) {
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

                    }
                    else{
                        n.y = (l.min.y + l.max.y) / 2;
                        n.x = l.max.x;
                        m.y = (l.min.y + l.max.y) / 2;
                        m.x = l.min.x;
                        
                    }
                    
                    if (l.p.x >= k.x && l.p.x <= n.x && l.p.y >= k.y && l.p.y <= n.y) {
                        l.max = n;
                        z1 = m;
                        z2 = o;
                    }
                    else{
                        l.min = m;
                        z1 = k;
                        z2 = n;
                    }
                    
                    while ((x_req < z1.x) || (x_req > z2.x) || (y_req < z1.y) || (y_req > z2.y)) {
                        x_req = rand() % z2.x;
                        y_req = rand() % z2.y;
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
                            prev = tmp;
                            tmp = tmp->next;
                        }else{
                            MPI_Send("delete -> OK", size, MPI_CHAR, tmp->id, TAG_DELETE, MPI_COMM_WORLD);
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
                
                prev = tmp;
                for (i = 0; i < l.nb_vois && tmp != NULL; i++, tmp = tmp->next) {
                    if (tmp->id == source) {
                        if (tmp == l.v) {
                            l.v = tmp->next;
                            free(tmp);
                            tmp = l.v;
                            break;
                        }else{
                            prev->next = tmp->next;
                            free(tmp);
                            tmp = prev->next;
                            break;
                        }
                    }
                }
                l.nb_vois--;
                break;
                
            case TAG_ADD:
                
                prev = l.v;
                tmp = (voisins *)malloc(sizeof(voisins));
                tmp->id = source;
                // car le mess sous forme xmin;ymin;xmax;ymax
                (tmp->min).x = atoi(strtok(data, ";"));;
                (tmp->min).y = atoi(strtok(NULL, ";"));;
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
                tmp->next = prev;
                l.v = tmp;
                
                break;
                
            case TAG_UPDATE:

                for (i = 0; i < l.nb_vois && tmp != NULL; i++, tmp = tmp->next) {
                    if (source == tmp->id) {
                        (tmp->min).x = atoi(strtok(data, ";"));
                        (tmp->min).y = atoi(strtok(NULL, ";"));
                        (tmp->max).x = atoi(strtok(NULL, ";"));
                        (tmp->max).y = atoi(strtok(NULL, ";"));
                        break;
                    }
                }
                break;
                
            case TAG_DATA:
                
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
                        struct donnee *d_tmp = l.d;
                        
                        do{
                            if (((d_tmp->p).x == x) && ((d_tmp->p).y == y)){
                                d_tmp->val = valeur;
                                break;
                            }
                            d_tmp = d_tmp->next;
                        }while(d_tmp != NULL);
                        
                        d_tmp = l.d;
                        d_tmp->prev = (struct donnee *)malloc(sizeof(struct donnee));
                        d_tmp->prev->next = d_tmp;
                        d_tmp->prev->prev = NULL;
                        l.d = d_tmp->prev;
                        l.d->val = valeur;
                        (l.d->p).x = x;
                        (l.d->p).y = y;
                    }
                    MPI_Send("ajout data -> OK", size, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
                }
                
                break;
            
            case TAG_SEARCH:
                
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
                            break;
                        }
                    }
                }else{
                    struct donnee *d_tmp = l.d;
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
        printf("my rankkkk = %d\n", my_rank);
        coordinateur(nb_proc);
    }else{
        printf("my rankkkk = %d\n", my_rank);

        noeud(my_rank);
    }
    
    MPI_Finalize();
    return 0;
}










