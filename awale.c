#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void afficherPlateau(int *plateau, char **joueur, int *points);
void jouer(int *plateau, char **joueur, int *points, int j);
int isTerrainAdverse(int j, int CaseChoisie);
int hasWin(int *points, char **joueur);

// Regle à dev : Si un coup devait prendre toutes les graines adverses, alors le coup peut être joué, mais aucune capture n'est faite : il ne faut pas « affamer » l'adversaire.

int main(int argc, char **argv)
{

    int *plateau = malloc(12 * sizeof(int));
    int *authorizedMove = malloc(12 * sizeof(int));
    int *points = malloc(2 * sizeof(int));
    int i;

    char *joueur[2] = {"Tim", "Hugo"};

    // initialisation du plateau
    for (i = 0; i < 12; i++)
    {
        plateau[i] = 2;
        authorizedMove[i] = 0;
    }

    // affichage du plateau
    afficherPlateau(plateau, joueur, points);

    // boucle de jeu
    while (1)
    {
        // joueur 1
        jouer(plateau, joueur, points, 0);
        afficherPlateau(plateau, joueur, points);

        // joueur 2
        jouer(plateau, joueur, points, 1);
        afficherPlateau(plateau, joueur, points);
    }
}

void jouer(int *plateau, char **joueur, int *points, int j)
{

    int caseChoisie = -1;

    printf("************************************ \r\n");

    while (caseChoisie < 0 || caseChoisie > 11)
    {
        if (j == 0)
        {
            printf("%s, choisissez une case (entre 0 et 5): ", joueur[j]);
            scanf("%d", &caseChoisie);
            printf("\r\n");
        }
        else
        {
            printf("%s, choisissez une case (entre 6 et 11): ", joueur[j]);
            scanf("%d", &caseChoisie);
            printf("\r\n");
        }

        /* VONT ÊTRE REMPLACÉ PAR AUTORIZEMOUV*/
        if (j == 0 && (caseChoisie < 0 || caseChoisie > 5))
        {
            printf("**/!\\ Case invalide !**\r\n");
        }
        else if (j == 1 && (caseChoisie < 6 || caseChoisie > 11))
        {
            printf("**/!\\ Case invalide !**\r\n");
        }

        if (plateau[caseChoisie] == 0)
        {
            printf("Case vide !\r\n");
        }
    }

    int graines = plateau[caseChoisie];
    plateau[caseChoisie] = 0;

    int caseInitiale = caseChoisie;

    while (graines > 0)
    {
        if (caseChoisie <= 0)
        {
            caseChoisie = 6;
        }
        else if (caseChoisie >= 11)
        {
            caseChoisie = 5;
        }
        else if (caseChoisie < 6)
        {
            caseChoisie--;
        }
        else if (caseChoisie > 5)
        {
            caseChoisie++;
        }

        if (caseChoisie == caseInitiale)
        {
            if (caseChoisie == 0)
            {
                caseChoisie = 6;
            }
            else if (caseChoisie == 11)
            {
                caseChoisie = 5;
            }
            else if (caseChoisie < 6)
            {
                caseChoisie--;
            }
            else if (caseChoisie > 5)
            {
                caseChoisie++;
            }
        }

        plateau[caseChoisie]++;
        graines--;
    }

    // sauvegarde du plateau et des points
    int pointsSave = points[j];
    int plateauSave[12];
    for (int i = 0; i < 12; i++)
    {
        plateauSave[i] = plateau[i];
    }

    // tant que la case d'arrivée est adverse et 1 < nbGraine < 4, on prend les graines
    while (isTerrainAdverse(j, caseChoisie) && plateau[caseChoisie] > 1 && plateau[caseChoisie] < 4)
    {
        printf("\033[1m%s prend les graines de la case %d\033[0m\r\n", joueur[j], caseChoisie);
        points[j] += plateau[caseChoisie];
        plateau[caseChoisie] = 0;

        if (j == 0)
        {
            caseChoisie--;
        }
        if (j == 1)
        {
            caseChoisie++;
        }
    }

    // si la prise de graine précedente entraine une stravation, on restore le plateau et les points comme il était avant
    if (hasStarvation(plateau))
    {
        printf("\033[1m%s a affamé son adversaire !\033[0m\r\n", joueur[j]);
        for (int i = 0; i < 12; i++)
        {
            plateau[i] = plateauSave[i];
        }
        points[j] = pointsSave;
    }
}

void afficherPlateau(int *plateau, char **joueur, int *points)
{
    printf("\e[1;33m\033[1mJoueur 1 : %s\033[0m\r\n", joueur[0]);
    printf("\e[0;35m\033[1mJoueur 2 : %s\033[0m\r\n", joueur[1]);

    printf("\r\n");

    int i = 0;
    printf("Case: –00– –01– –02– –03– –04– –05– \r\n");
    printf("––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
    printf("\e[1;33m\033[1mJ1 :  ");
    for (i = 0; i < 6; i++)
    {
        if (plateau[i] < 10)
        {
            printf("| 0%d ", plateau[i]);
        }
        else
        {
            printf("| %d ", plateau[i]);
        }
    }
    printf("|     %d pts\033[0m\r\n", points[0]);
    printf("––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");

    printf("\e[0;35m\033[1mJ2 :  ");
    for (i = 6; i < 12; i++)
    {
        if (plateau[i] < 10)
        {
            printf("| 0%d ", plateau[i]);
        }
        else
        {
            printf("| %d ", plateau[i]);
        }
    }

    printf("|     %d pts\033[0m\r\n", points[1]);
    printf("––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
    printf("Case: –06– –07– –08– –09– –10– –11– \r\n\r\n");
}

int hasStarvation(int *plateau)
{
    int i, offset, sumJ1 = 0, sumJ2 = 0;

    // Calcul la somme des graines de chaque joueur
    for (i = 0; i < 6; i++)
    {
        sumJ1 += plateau[i];
    }
    for (i = 6; i < 12; i++)
    {
        sumJ2 += plateau[i];
    }

    // Vérifie si un joueur n'a plus de graines
    if (sumJ1 == 0)
    {
        offset = 6;
    }
    else if (sumJ2 == 0)
    {
        offset = 0;
    }
    else
    {
        return 0;
    }
    // Verfie s'il y a une famine
    for (i = 0; i < 6; i++)
    {
        if (plateau[i + offset] > 6 - i && plateau[i + offset] >= 4)
        {
            return 0;
        }
    }
    return 1;
}

int hasWin(int *points, char **joueur)
{
    if (points[0] < points[1])
    {
        printf("%s a gagné !\r\n", joueur[1]);
        return 0;
    }
    else if (points[0] > points[1])
    {
        printf("%s a gagné !\r\n", joueur[0]);
        return 1;
    }
    else
    {
        printf("Egalité !\r\n");
    }

    return 2;
}

int isTerrainAdverse(int j, int caseChoisie)
{
    int tableauJ1[] = {0, 1, 2, 3, 4, 5};
    int tableauJ2[] = {6, 7, 8, 9, 10, 11};

    if (j == 0)
    {
        for (int i = 0; i < 6; i++)
        {
            // printf("J%d – %d – %d\r\n", j, plateau[tableauJ2[i]], caseChoisie);
            if (tableauJ2[i] == caseChoisie)
            {
                // printf("Fini terrain adverse\r\n");
                return 1;
            }
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            // printf("J%d – %d – %d", j, plateau[tableauJ1[i]], caseChoisie);
            if (tableauJ1[i] == caseChoisie)
            {
                // printf("Fini terrain adverse\r\n");
                return 1;
            }
        }
    }

    return 0;
}