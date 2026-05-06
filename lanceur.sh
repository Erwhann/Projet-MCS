# https://linuxvox.com/blog/zenity-linux/

#!/bin/bash
CHOIX=$(zenity --list --radiolist \
    --title="Dashboard - Puissance 2i" \
    --text="Que voulez-vous lancer ?" \
    --column="Choix" --column="Action" \
    TRUE "1. Démarrer le Serveur" \
    FALSE "2. Lancer le Client" \
    --width=550 --height=550)

if [ -z "$CHOIX" ]; then
    exit 0
fi

if [ "$CHOIX" = "1. Démarrer le Serveur" ]; then
    ./bin/serveur &
    zenity --info --title="Succès" --text="Le serveur a été lancé en arrière-plan." --width=200

elif [ "$CHOIX" = "2. Lancer le Client" ]; then
    IP=$(zenity --entry \
        --title="Configuration Client" \
        --text="Entrez l'adresse IP du serveur :" \
        --entry-text="127.0.0.1" \
        --width=500)
    
    if [ -n "$IP" ]; then
        gnome-terminal -- bash -c "./bin/client $IP; exec bash"
    fi
fi

 

