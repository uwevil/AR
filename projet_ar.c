#include "projet_ar.h"

void coordinateur(){
    char *data;
    MPI_Status status;
    int source;
    
    while (1) {
        MPI_Recv(data, 128, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch (status.MPI_TAG) {
            case 0:
                source = status.MPI_SOURCE;
                MPI_Send(data, 128, MPI_CHAR, 1, TAG_NOEUD, MPI_COMM_WORLD);
                MPI_Recv(data, 128, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD, &status);
                break;
            
                
                
            default:
                break;
        }
    }
}

int est_voisins(point a, point b, point c, point d){
    if ((a.x == c.x) || (b.x == d.x) || (a.x == d.x) || (b.x == c.x)) {
        //y_max_a = b.y; y_max_c = d.y
        //y_min_a = a.y; y_min_c = c.y
        if ((a.y =< c.y) && (d.y <= b.y) || (a.y >= c.y) && (b.y <= d.y)){
            return 1;
        }
    }
    if ((a.y == c.y) || (b.y == d.y) || (a.y == d.y) || (b.y == c.y)) {
        if ((a.x =< c.x) && (d.x <= b.x) || (a.x >= c.x) && (b.x <= d.x)){
            return 1;
        }
    }
    
    return 0;
}

void noeud(int rang){
    struct local l;
    char *data;
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
        L.v = NULL;
    }
    else{
        /*
         x;y;xmin;ymin;xmax;ymax;nb-voisins;id1;xmin1;ymin1;xmax1;ymax1;id2....
         */
        l.p.x = rand() % 1000;
        l.p.y = rand() % 1000;
        sprintf(data, "%d;%d;%d\n", rang, l.p.x, l.p.y);
        MPI_Send(data, 128, MPI_CHAR, 0, TAG_NOEUD, MPI_COMM_WORLD);
        MPI_Recv(data, 128, MPI_CHAR, MPI_ANY_SOURCE, TAG_OK, MPI_COMM_WORLD, &status);
    
        l.p.x = atoi(strtok(data, ";"));
        l.p.y = atoi(strtok(NULL, ";"));
        l.min.x = atoi(strtok(NULL, ";"));
        l.min.y = atoi(strtok(NULL, ";"));
        l.max.x = atoi(strtok(NULL, ";"));
        l.max.y = atoi(strtok(NULL, ";"));
        l.nb_vois = atoi(strtok(NULL, ";"));
    
        int i;
        voisins *tmp;
        char *buf, *buf_tmp1, *buf_tmp2;
    
        if (l.nb_vois > 0) {
            l.v = (voisins *)malloc(sizeof(voisins));
            voisins *tmp = l.v;
            
            sprintf(buf, "%d;%d;%d\n", rang, l.x, l.y);

            for (i = 0; i < l.nb_vois; i++) {
                tmp->id = strtok(NULL, ";");
                
                MPI_Send(buf, 128, MPI_CHAR, tmp->id, TAG_UPDATE, MPI_COMM_WORLD);
                
                tmp->min.x = atoi(strtok(NULL, ";"));
                tmp->min.y = atoi(strtok(NULL, ";"));
                tmp->max.x = atoi(strtok(NULL, ";"));
                tmp->max.y = atoi(strtok(NULL, ";"));
                
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
        MPI_Send("", 128, MPI_CHAR, 0, TAG_OK, MPI_COMM_WORLD);
    }

    /**ecoute**/
    while (1) {
        MPI_Recv(data, 128, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;
        int id_req = atoi(strtok(data, ";"));
        int x_req = atoi(strtok(NULL, ";"));
        int y_req = atoi(strtok(NULL, ";"));
        
        int i, t;
        int route;
        voisins *tmp = l.v;
        voisind *prev;
        point k, m, n, o;
        point z1, z2;
        
        switch (status.MPI_TAG) {
            case TAG_NOEUD:
                if ((x_req < l.min.x) || (x_req > l.max.x) || (y_req < l.min.y) || (y_req > l.max.y)) {
                    if (x_req >= y_req) {
                        if (x_req > l.max.x) {
                            route = 1;
                        }else {
                            route = 3;
                        }
                    }else{
                        if (y_req > l.max.y) {
                            route = 0;
                        }else{
                            route = 2;
                        }
                    }
                    for (i = 0; i < l.nb_vois; i ++) {
                        if (tmp->pos == route) {
                            sprintf(buf, "%d;%d;%d\n", id_req, x_req, y_req);
                            MPI_Send(buf, 128, MPI_CHAR, tmp->id, TAG_NOEUD, MPI_COMM_WORLD);
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
                        z2 = o
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
                    for (i = 0; i < l.nb_vois; i++) {
                        if (est_voisins(z1, z2, tmp->min, tmp->max)){
                            t++;
                            sprintf(buf_tmp2, ";%d;%d;%d;%d;%d", tmp->id, tmp->min.x, tmp->min.y, tmp->max.x, tmp->max.y);
                            strcat(buf_tmp1, buf_tmp2);
                        }
                        if (est_voisins(l.min, l.max, tmp->min, tmp->max)) {
                            sprintf(buf, "%d;%d;%d;%d;%d\n", l.id, l.min.x, l.min.y, l.max.x, l.max.y);
                            MPI_Send(buf, 128, MPI_CHAR, tmp->id, TAG_UPDATE, MPI_COMM_WORLD);
                            prev = tmp;
                            tmp = tmp->next;
                        }else{
                            MPI_Send("", 128, MPI_CHAR, tmp->id, TAG_DELETE, MPI_COMM_WORLD);
                            if (tmp == l.v) {
                                l.v = tmp->next;
                                free(tmp);
                                tmp = l.v;
                                prev = l.v;
                            }else{
                                prev->next = tmp->next;
                                free(tmp);
                                tmp = prev->next;
                            }
                        }
                    }
                    
                    sprintf(buf, "%d;%d;%d;%d;%d;%d;%d", x_req, y_req, z1.x, z1.y, z2.x, z2.y, t);
                    strcat(buf, buf_tmp1);
                    buf[strlen(buf)] = '\0';
                    
                    MPI_Send(buf, 128, MPI_CHAR, source, TAG_OK, MPI_COMM_WORLD);
                }
                break;
            case
                    
        
                
            default:
                break;
        }
        
    }
}










