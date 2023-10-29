# **Jeu du AWALE en réseau**

##### _Auteurs : Hugo WARIN et Tim MOREL_

### **Compilation**

Pour compiler le programme, il suffit de taper la commande "make" dans le terminal.
Cela produit deux executables : **"server_bin"** et **"client_bin"**.

### **Lancement du Serveur**

La première étape est de lancer le serveur. Pour cela, il faut lancer la commande **"./server_bin"**.
Ensuite il suffit que des clients se connectent au serveur pour pouvoir jouer.

### **Lancement du Client**

Pour lancer un client, il faut lancer la commande **"./client_bin <adresse IP du serveur> <Pseudo>"**.
Vous pouvez répeter ce opération pour autant de client que voulu (dans la limite de 100 clients).

### **Jouer !**

Une fois le client lancé, il suffit de suivre les instructions affichées à l'écran pour jouer.

# **Fonctionnalités implémentées**

Voici les fonctionnalités développées :

- **Jouer** au jeu de l'AWALE en réseau, **en défiant son adversaire** grâce à son pseudo (cet le joueur adverse doit être en attente dans la Waiting-list).
- Lorsqu'un joueur est défié, il peut **accepter ou refuser le challenge**.
- **Observer** une partie en cours.
- **Tchatter** avec les autres joueurs ; le chat est commun pour tout le monde lorsque le joueur n'est pas en train de jouer. Sinon, le chat ne concerne que la partie en question (joueurs et observateurs).
- Chaque joueur peut **éditer ou consulter une biographie**.
- **Consulter la liste des joueurs** inscrits et des joueurs en ligne.
- La **déconnexion à tout moment de la partie** est _théoriquement_ gérée par le serveur.
