#ifndef AWALE_H
#define AWALE_H

#include "Serveur/client2.h"

void afficherPlateau(int *plateau, char **joueur, int *points);
void jouer(Client *client, int *plateau, char **joueur, int *authorizedMove, int *points, int j);
int isTerrainAdverse(int j, int CaseChoisie);
int updateAuthorizedMove(int *authorizedMove, int *plateau, int j);
int checkStarvation(int *plateau, int *authorizedMove, int j);
int hasWin(int *points, char **joueur);
int isFinish(Client **clients, int *plateau, int *points, int *authorizedMove, int *joueur);

#endif