#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void afficherPlateau(int *plateau, char **joueur);
void jouer(int *plateau, char **joueur, int j);
int hasWin();

int main(int argc, char **argv)
{

    int *plateau = malloc(12 * sizeof(int));
    int i;

    char *joueur[2] = {"Tim", "Hugo"};

    // initialisation du plateau
    for (i = 0; i < 12; i++)
    {
        plateau[i] = 4;
    }

    // affichage du plateau
    afficherPlateau(plateau, joueur);

    // boucle de jeu
    while (hasWin() == 0)
    {
        // joueur 1
        jouer(plateau, joueur, 0);
        afficherPlateau(plateau, joueur);

        // joueur 2
        jouer(plateau, joueur, 1);
        afficherPlateau(plateau, joueur);
    }
}

void jouer(int *plateau, char **joueur, int j)
{
    int caseChoisie = -1;
    printf("************************************ \r\n");

    while (caseChoisie < 0 || caseChoisie > 5)
    {
        printf("%s, choisissez une case (entre 0 et 5): ", joueur[j]);
        scanf("%d", &caseChoisie);
        printf("\r\n");
    }

    caseChoisie = caseChoisie + 6 * j;

    int graines = plateau[caseChoisie];
    plateau[caseChoisie] = 0;

    while (graines > 0)
    {
        if (caseChoisie < 6)
        {
            caseChoisie = (caseChoisie - 1);
        }
        else if (caseChoisie > 5)
        {
            caseChoisie = (caseChoisie + 1);
        }

        plateau[caseChoisie]++;
        graines--;
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

    printf("|\r\n\r\n");
}

int hasWin()
{
    // true (!= 0) si un des joueurs a gagné
    return 0;
}
