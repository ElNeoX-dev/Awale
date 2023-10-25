#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "awale.h"

/*
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

    for (i = 6; i < 12; i++)
    {
        plateau[i] = 0;
    }

    plateau[6] = 1;
    plateau[7] = 1;

    // affichage du plateau
    afficherPlateau(plateau, joueur, points);

    // boucle de jeu
    while (isFinish(plateau, points, authorizedMove, joueur) == 0)
    {
        // joueur 1
        jouer(plateau, joueur, authorizedMove, points, 0);
        afficherPlateau(plateau, joueur, points);

        // joueur 2
        jouer(plateau, joueur, authorizedMove, points, 1);
        afficherPlateau(plateau, joueur, points);
    }
}

int isFinish(int *plateau, int *points, int *authorizedMove, int *joueur)
{
    int i, sumJ1 = 0, sumJ2 = 0;

    // Calculate the sum of seeds for each player
    for (i = 0; i < 6; i++)
    {
        sumJ1 += plateau[i];
    }
    for (i = 6; i < 12; i++)
    {
        sumJ2 += plateau[i];
    }

    // Check if either player has won
    if ((sumJ1 == 0 || sumJ2 == 0) && (checkStarvation(plateau, authorizedMove, 0) == 0) && (checkStarvation(plateau, authorizedMove, 1) == 0))
    {
        // The game is over, so distribute remaining seeds to the winner's side
        for (i = 0; i < 6; i++)
        {
            points[1] += plateau[i];
            plateau[i] = 0;
        }
        for (i = 6; i < 12; i++)
        {
            points[0] += plateau[i];
            plateau[i] = 0;
        }

        // Determine the winner
        if (points[0] > points[1])
        {
            printf("%s wins!\n", joueur[0]);
        }
        else if (points[1] > points[0])
        {
            printf("%s wins!\n", joueur[1]);
        }
        else
        {
            printf("It's a tie!\n");
        }

        return 1;
    }

    return 0;
}

int updateAuthorizedMove(int *authorizedMove, int *plateau, int j)
{
    if (checkStarvation(plateau, authorizedMove, j) == 1)
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
    int validMove = 0;
    printf("************************************ \r\n");
    if (updateAuthorizedMove(authorizedMove, plateau, j) == 1)
    {
        printf("\e[0;31m\033[1mStarvation !\033[0m\r\n");
        printf("\e[0;32m\033[1mCases possibles : ");
        for (i = 0; i < 6; i++)
        {
            if (authorizedMove[i] == 1)
            {
                printf("%d ", i + j * 6);
            }
        }
        printf("\033[0m\r\n");
    }
    while (validMove != 1)
    {
        if (j == 0)
        {
            printf("%s, choisissez une case non-vide (entre 0 et 5): ", joueur[j]);
            scanf("%d", &caseChoisie);
            printf("\r\n");
        }
        else
        {
            printf("%s, choisissez une case non-vide (entre 6 et 11): ", joueur[j]);
            scanf("%d", &caseChoisie);
            printf("\r\n");
        }

        if (caseChoisie < j * 6 + 6 && caseChoisie >= j * 6)
        {
            if (authorizedMove[caseChoisie - j * 6] == 0)
            {
                printf("\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n");
                validMove = 0;
            }
            else
            {
                validMove = 1;
            }
        }
        else
        {
            printf("\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n");
            validMove = 0;
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
    if (checkStarvation(plateau, authorizedMove, j))
    {
        printf("\033[1m%s a affamé son adversaire ! Il ne prend donc pas les graines...\033[0m\r\n", joueur[j]);
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
    printf("Case:  –00– –01– –02– –03– –04– –05– \r\n");
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
    printf("Case:  –06– –07– –08– –09– –10– –11– \r\n\r\n");
}

int checkStarvation(int *plateau, int *authorizedMove, int j)
{
    int i, sum = 0;
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
*/

int isFinish(Game *game, char *message)
{
    int i, sumJ1 = 0, sumJ2 = 0;
    char buffer[1024];
    // Calculate the sum of seeds for each player
    for (i = 0; i < 6; i++)
    {
        sumJ1 += game->plateau[i];
    }
    for (i = 6; i < 12; i++)
    {
        sumJ2 += game->plateau[i];
    }

    // Check if either player has won
    if ((sumJ1 == 0 || sumJ2 == 0) && (checkStarvation(game, 0) == 0) && (checkStarvation(game, 1) == 0))
    {
        // The game is over, so distribute remaining seeds to the winner's side
        for (i = 0; i < 6; i++)
        {
            game->points[1] += game->plateau[i];
            game->plateau[i] = 0;
        }
        for (i = 6; i < 12; i++)
        {
            game->points[0] += game->plateau[i];
            game->plateau[i] = 0;
        }

        // Determine the winner
        if (game->points[0] > game->points[1])
        {
            sprintf(buffer, VERT BOLD "%s wins!\r\n" RESET, game->clients[0]->name);
            strcat(message, buffer);
        }
        else if (game->points[1] > game->points[0])
        {
            sprintf(buffer, VERT BOLD "%s wins!\r\n" RESET, game->clients[1]->name);
            strcat(message, buffer);
        }
        else
        {
            sprintf(buffer, VERT BOLD "It's a tie!\n" RESET);
            strcat(message, buffer);
        }

        return 1;
    }

    return 0;
}

int updateAuthorizedMove(Game *game, int j)
{
    if (checkStarvation(game, j) == 1)
    {
        return 1;
    }
    for (int i = 0; i < 6; i++)
    {
        game->authorizedMove[i] = (game->plateau[i + j * 6] != 0);
    }
    return 0;
}

int jouer(Game *game, int j, int caseChoisie, char *message)
{
    printf("caseChoisie = %d\r\n", caseChoisie);
    int i;
    int coupJoue = caseChoisie;
    char textBuffer[1024];
    int validMove = 0;
    // message = "";
    strcat(message, "************************************ \r\n");
    if (updateAuthorizedMove(game, j) == 1)
    {
        strcat(message, "\e[0;31m\033[1mStarvation !\033[0m\r\n");
        strcat(message, "\e[0;32m\033[1mCases possibles : ");
        // write_client(game->clients[j]->sock, "\e[0;31m\033[1mStarvation !\033[0m\r\n");
        // write_client(game->clients[j]->sock, "\e[0;32m\033[1mCases possibles : ");
        for (i = 0; i < 6; i++)
        {
            if (game->authorizedMove[i] == 1)
            {
                sprintf(textBuffer, "%d ", i + j * 6);
                strcat(message, textBuffer);
            }
        }
        strcat(message, "\033[0m\r\n");
        // write_client(game->clients[j]->sock, "\033[0m\r\n");
    }
    // while (validMove != 1)
    // {
    // if (j == 0)
    // {

    //     sprintf(textBuffer, "%s, choisissez une case non-vide (entre 0 et 5): ", game->clients[j]);
    //     strcat(message, textBuffer);
    //     // write_client(game->clients[j]->sock, "%s, choisissez une case non-vide (entre 0 et 5): ", game->clients[j]);
    //     // scanf("%d", &caseChoisie);
    //     // write_client(game->clients[j]->sock, "\r\n");
    //     return -1;
    // }
    // else
    // {
    //     sprintf(textBuffer, "%s, choisissez une case non-vide (entre 6 et 11): ", game->clients[j]);
    //     strcat(message, textBuffer);
    //     // write_client(game->clients[j]->sock, "%s, choisissez une case non-vide (entre 6 et 11): ", game->clients[j]);
    //     // scanf("%d", &caseChoisie);
    //     // write_client(game->clients[j]->sock, "\r\n");
    //     return -1;
    // }

    if (caseChoisie < j * 6 + 6 && caseChoisie >= j * 6)
    {
        if (game->authorizedMove[caseChoisie - j * 6] == 0)
        {
            strcat(message, "\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n");
            // write_client(game->clients[j]->sock, "\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n");
            return -2;
        }
    }
    else
    {
        strcat(message, "\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n");
        // write_client(game->clients[j]->sock, "\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n");
        return -2;
    }
    // }

    int graines = game->plateau[caseChoisie];
    game->plateau[caseChoisie] = 0;

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

        game->plateau[caseChoisie]++;
        graines--;
    }

    // sauvegarde du plateau et des points
    int pointsSave = game->points[j];
    int plateauSave[12];
    for (int i = 0; i < 12; i++)
    {
        plateauSave[i] = game->plateau[i];
    }

    // tant que la case d'arrivée est adverse et 1 < nbGraine < 4, on prend les graines
    while (isTerrainAdverse(j, caseChoisie) && game->plateau[caseChoisie] > 1 && game->plateau[caseChoisie] < 4)
    {
        sprintf(textBuffer, "\033[1m%s prend les graines de la case %d\033[0m\r\n", game->clients[j]->name, caseChoisie);
        strcat(message, textBuffer);
        // write_client(game->clients[j]->sock, "\033[1m%s prend les graines de la case %d\033[0m\r\n", game->clients[j], caseChoisie);
        game->points[j] += game->plateau[caseChoisie];
        game->plateau[caseChoisie] = 0;

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
    if (checkStarvation(game, j))
    {
        sprintf(textBuffer, "\033[1m%s a affamé son adversaire ! Il ne prend donc pas les graines...\033[0m\r\n", game->clients[j]->name);
        strcat(message, textBuffer);
        // write_client(game->clients[j]->sock, "\033[1m%s a affamé son adversaire ! Il ne prend donc pas les graines...\033[0m\r\n", game->clients[j]);
        for (int i = 0; i < 12; i++)
        {
            game->plateau[i] = plateauSave[i];
        }
        game->points[j] = pointsSave;
        return -3;
    }
    return coupJoue;
}

int checkStarvation(Game *game, int j)
{
    int i, sum = 0;
    int starvation = 0;

    if (j == 0)
    {
        for (i = 6; i < 12; i++)
        {
            sum += game->plateau[i];
        }
        if (sum == 0)
        {
            for (i = 0; i < 6; i++)
            {
                if (game->plateau[i] > 1 + i && game->plateau[i] >= 4)
                {
                    game->authorizedMove[i] = 1;
                    starvation = 1;
                }
                else
                {
                    game->authorizedMove[i] = 0;
                }
            }
        }
    }
    else if (j == 1)
    {
        for (i = 0; i < 6; i++)
        {
            sum += game->plateau[i];
        }
        if (sum == 0)
        {
            for (i = 0; i < 6; i++)
            {
                if (game->plateau[i + 6] > 6 - i && game->plateau[i + 6] >= 4)
                {
                    game->authorizedMove[i] = 1;
                    starvation = 1;
                }
                else
                {
                    game->authorizedMove[i] = 0;
                }
            }
        }
    }
    return starvation;
}

/*
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
*/

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

void genererAffPlateau(Game *game, char *affichagePlateau)
{
    char buffer[1024];
    sprintf(buffer, JAUNE BOLD "Joueur 1 : %s\r\n" RESET, game->clients[0]->name);
    strcat(affichagePlateau, buffer);
    sprintf(buffer, VIOLET BOLD "Joueur 2 : %s\r\n" RESET, game->clients[1]->name);
    strcat(affichagePlateau, buffer);

    sprintf(buffer, "\r\n");
    strcat(affichagePlateau, buffer);

    int i = 0;
    sprintf(buffer, "Case:  –00– –01– –02– –03– –04– –05– \r\n");
    strcat(affichagePlateau, buffer);
    sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
    strcat(affichagePlateau, buffer);
    sprintf(buffer, JAUNE BOLD "J1 :  ");
    strcat(affichagePlateau, buffer);
    for (i = 0; i < 6; i++)
    {
        if (game->plateau[i] < 10)
        {
            sprintf(buffer, "| 0%d ", game->plateau[i]);
            strcat(affichagePlateau, buffer);
        }
        else
        {
            sprintf(buffer, "| %d ", game->plateau[i]);
            strcat(affichagePlateau, buffer);
        }
    }
    sprintf(buffer, "|     %d pts\r\n" RESET, game->points[0]);
    strcat(affichagePlateau, buffer);
    sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
    strcat(affichagePlateau, buffer);

    sprintf(buffer, VIOLET BOLD "J2 :  ");
    strcat(affichagePlateau, buffer);
    for (i = 6; i < 12; i++)
    {
        if (game->plateau[i] < 10)
        {
            sprintf(buffer, "| 0%d ", game->plateau[i]);
            strcat(affichagePlateau, buffer);
        }
        else
        {
            sprintf(buffer, "| %d ", game->plateau[i]);
            strcat(affichagePlateau, buffer);
        }
    }

    sprintf(buffer, "|     %d pts\r\n" RESET, game->points[1]);
    strcat(affichagePlateau, buffer);
    sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
    strcat(affichagePlateau, buffer);
    sprintf(buffer, "Case:  –06– –07– –08– –09– –10– –11– \r\n\r\n");
    strcat(affichagePlateau, buffer);
}

static void write_client(SOCKET sock, const char *message, ...)
{
    va_list args;
    va_start(args, message);

    // Créez un tampon pour formater le message
    char buffer[1024]; // Vous pouvez ajuster la taille du tampon en fonction de vos besoins

    // Formatez le message en utilisant vsnprintf
    vsnprintf(buffer, sizeof(buffer), message, args);

    // Affichez le message à la sortie standard
    printf("%s", buffer);

    if (send(sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        va_end(args);
        exit(errno);
    }
    va_end(args);
}

static void write_to_players(Client **clients, const char *message, ...)
{
    va_list args;
    va_start(args, message);

    // Créez un tampon pour formater le message
    char buffer[1024]; // Vous pouvez ajuster la taille du tampon en fonction de vos besoins

    // Formatez le message en utilisant vsnprintf
    vsnprintf(buffer, sizeof(buffer), message, args);

    int i = 0;
    // a changer en fonction du nombre d'observateurs
    for (i = 0; i < 2; i++)
    {
        write_client(clients[i]->sock, buffer);
    }
    va_end(args);
}
