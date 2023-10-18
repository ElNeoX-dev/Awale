#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void afficherPlateau(int *plateau, char **joueur);
void jouer(int *plateau, char **joueur, int *authorizedMove, int *points, int j);
int aFiniTerrainAdverse(int j, int *plateau, int CaseChoisie);
int updateAuthorizedMove(int *authorizedMove, int *plateau, int j);
int hasWin(int *points, char **joueur);
int isFinish(int *plateau, int *points);

int main(int argc, char **argv)
{

    int *plateau = malloc(12 * sizeof(int));
    int *authorizedMove = malloc(6 * sizeof(int));
    int *points = malloc(2 * sizeof(int));
    int i;

    char *joueur[2] = {"Tim", "Hugo"};

    // initialisation du plateau
    for (i = 0; i < 12; i++)
    {
        plateau[i] = 4;
        authorizedMove[i] = 0;
    }

    // affichage du plateau
    afficherPlateau(plateau, joueur);

    // boucle de jeu
    while (isFinish() == 0)
    {
        // joueur 1
        jouer(plateau, joueur, points, 0);
        afficherPlateau(plateau, joueur);

        // joueur 2
        jouer(plateau, joueur, points, 1);
        afficherPlateau(plateau, joueur);
    }
}

int updateAuthorizedMove(int *authorizedMove, int *plateau, int j)
{
    if(checkStarvation(plateau, authorizedMove, j) == 1)
    {
        return 1;
    }
    for (int i = 0; i < 6; i++)
    {
        authorizedMove[i] = (plateau[i + j * 6] != 0);
    }
    return 0;
}

void jouer(int *plateau, char **joueur, int *authorizedMove, int *points, int j)
{
    int i;
    int caseChoisie = -1;
    int notAuthorizedMove = 1;
    printf("************************************ \r\n");
    if(updateAuthorizedMove(authorizedMove, plateau, j))
    {
        printf("\e[0;35mStarvation !\033[0m\r\n");
        printf("Cases possibles : ");
        for(i = 0; i < 6; i++)
        {
            if(authorizedMove[i] == 1)
            {
                printf("%d ", i + j * 6);
            }
        }
        printf("\r\n");
    }
    while (notAuthorizedMove == 1)
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

        if (authorizedMove[caseChoisie] == 0)
        {
            printf("**/!\\ Case vide !**\r\n");
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

int checkStarvation(int *plateau, int *authorizedMove, int j)
{
    int i, offset, sum = 0;
    int starvation = 0;

    if (j == 0)
    {
        for (i = 6; i < 12; i++)
        {
            sum += plateau[i];
        }
        if (sum == 0)
        {
            for (i = 0; i < 6; i++)
            {
                if (plateau[i] > 1 + i && plateau[i] >= 4)
                {
                    authorizedMove[i] = 1;
                    starvation = 1;
                }
                else 
                {
                    authorizedMove[i] = 0;
                }
            }
        }
    }
    else if (j == 1)
    {
        for (i = 0; i < 6; i++)
        {
            sum += plateau[i];
        }
        if (sum == 0)
        {
            for (i = 0; i < 6; i++)
            {
                if (plateau[i + 6] > 6 - i && plateau[i + 6] >= 4)
                {
                    authorizedMove[i] = 1;
                    starvation = 1;
                }
                else 
                {
                    authorizedMove[i] = 0;
                }
            }
        }
    }
    return starvation;
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