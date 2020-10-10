#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Arduino.h"

/*Description electrique, issue de l'ancien code
//Boutons
--> branchés en PULLUP interne, etat actif bas
btn ouverture manuelle --> 2
btn fermeture manuelle --> 6

//Capteurs
capteur hall courant moteur --> A2
    --> min ouverture 350
    --> max ouverture 690
    --> "position neutre" 511

photorésistance --> A0
    --> seuil diurne 85
    --> seuil nocturne 150

butees, actives à 0, INPUT_PULLUP
capteur porte ouverte --> 11
capteur porte fermee --> 12

//Sorties moteur
PWM, réglé à 250 (pas 255 tant qu'à faire ?)
--> 10
--> 5
*/

//options
#define DEBUG_SERIAL true
#define TIMEOUT_MOTEUR_PORTE 6000 //timeout de securite = tps maximal d'activation du moteur (millisecondes)

#define UTILISER_MESURE_COURANT false
#define COURANT_MAX 690

#define CAPTEUR_ACTIF 0 //Les butees sont actives a 0
#define BTN_ACTIF 0

#define VITESSE_MOTEUR 250

//branchements
#define PIN_COMMANDE_MOTEUR_OUVERTURE 10
#define PIN_COMMANDE_MOTEUR_FERMETURE 5
#define PIN_CAPTEUR_PORTE_OUVERTE 11
#define PIN_CAPTEUR_PORTE_FERMEE 12
#define PIN_CAPTEUR_COURANT_MOTEUR A2
#define PIN_BTN_OUVERTURE_MANUELLE 2
#define PIN_BTN_FERMETURE_MANUELLE 6


#endif
