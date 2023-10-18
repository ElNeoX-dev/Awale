#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void afficherPlateau(int *plateau, char **joueur);
void jouer(int *plateau, char **joueur, int *points, int j);
int aFiniTerrainAdverse(int j, int *joueur, int CaseChoisie);
int hasWin();

int main(int argc, char **argv)
{

    int *plateau = malloc(12 * sizeof(int));
<<<<<<< HEAD
    int *authorizedMove = malloc(12 * sizeof(int));
=======
    int *points = malloc(2 * sizeof(int));
>>>>>>> main
    int i;

    char *joueur[2] = {"Tim", "Hugo"};

    // initialisation du plateau
    for (i = 0; i < 12; i++)
    {
<<<<<<< HEAD
        plateau[i] = 4;
        authorizedMove[i] = 0;
=======
        plateau[i] = 1;
>>>>>>> main
    }

    // affichage du plateau
    afficherPlateau(plateau, joueur);

    // boucle de jeu
    while (hasWin() == 0)
    {
        // joueur 1
        jouer(plateau, joueur, points, 0);
        afficherPlateau(plateau, joueur);

        // joueur 2
        jouer(plateau, joueur, points, 1);
        afficherPlateau(plateau, joueur);
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

    // si la case d'arrivée est adverse et 1 < nbGraine < 4, on prend les graines
    if (aFiniTerrainAdverse(j, plateau, caseChoisie) && plateau[caseChoisie] > 1 && plateau[caseChoisie] < 4)
    {
        // printf("Vous avez pris les graines de la case %d !\r\n", caseChoisie);
        // plateau[caseChoisie] = 0;
        printf("Je peu prendre la case %d", caseChoisie);
    }
}

void afficherPlateau(int *plateau, char **joueur)
{
    printf("Joueur 1 : %s\r\n", joueur[0]);
    printf("Joueur 2 : %s\r\n", joueur[1]);

    printf("\r\n");

    int i = 0;
    printf("Case: –00– –01– –02– –03– –04– –05– \r\n");
    printf("     ––––––––––––––––––––––––––––––– \r\n");
    printf("J1 : ");
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
    printf("|\r\n");
    printf("      –––– –––– –––– –––– –––– –––– \r\n");

    printf("J2 : ");
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

    printf("|\r\n");
    printf("     ––––––––––––––––––––––––––––––– \r\n");
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
    //Verfie s'il y a une famine
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
    if(points[0] < points[1])
    {
        printf("%s a gagné !\r\n", joueur[1]);
        return 0;
    }
    else if(points[0] > points[1])
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

int aFiniTerrainAdverse(int j, int *plateau, int caseChoisie)
{
    int tableauJ1[] = {0, 1, 2, 3, 4, 5};
    int tableauJ2[] = {6, 7, 8, 9, 10, 11};

    if (j == 0)
    {
        for (int i = 0; i < 6; i++)
        {
            if (plateau[tableauJ2[i]] == caseChoisie)
            {
                printf("terrain adverse");
                return 1;
            }
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            if (plateau[tableauJ1[i]] == caseChoisie)
            {
                printf("terrain adverse");
                return 1;
            }
        }
    }

    return 0;
}