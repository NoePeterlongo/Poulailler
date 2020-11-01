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

/*Procédure jour/nuit:

------LEVER-MARGE_PRELEVER*****LEVER+MARGE_POSTLEVER+++++COUCHER-MARGE_PRECOUCHER****COUCHER+MARGE_POSTCOUCHER-----

----    ->  Nuit            -> porte fermee
****    ->  Aube/crepuscule -> La luminosite decide
++++    ->  jour            -> porte ouverte
LEVER/COUCHER   -> definis par RTC et calcul
MARGES          -> definies ci-dessous

Luminosite -> Une fois un seuil passe, on attend DELAI_LUMINOSITE avant de verifier et de prendre une decision

*/

//options
#define DEBUG_SERIAL true
#define TIMEOUT_MOTEUR_PORTE 6000 //timeout de securite = tps maximal d'activation du moteur (millisecondes)
#define VITESSE_MOTEUR 250

#define UTILISER_MESURE_COURANT false
#define COURANT_MAX 690

//detection du jour
#define SEUIL_MATINAL       85//200
#define SEUIL_VESPERAL      150//500
#define SEUIL_ERREUR_JOUR   900
#define SEUIL_ERREUR_NUIT   100
#define DELAI_LUMINOSITE    600000L
#define DELAI_LUMINOSITE_VERIFICATION_ERREUR    600000L
#define PERIODE_MESURE_LUMINOSITE 1000 //pour la moyenne
#define MARGE_PRELEVER      15//Les marges sont en minutes
#define MARGE_POSTLEVER     45
#define MARGE_PRECOUCHER    30
#define MARGE_POSTCOUCHER   45



//Définit l'état actif des capteurs
#define CAPTEUR_ACTIF 0 //Les butees sont actives a 0
#define BTN_ACTIF 0


//branchements
#define PIN_COMMANDE_MOTEUR_OUVERTURE 10
#define PIN_COMMANDE_MOTEUR_FERMETURE 5
#define PIN_CAPTEUR_PORTE_OUVERTE 11
#define PIN_CAPTEUR_PORTE_FERMEE 12
#define PIN_CAPTEUR_COURANT_MOTEUR A2
#define PIN_PHOTORESISTANCE A0
#define PIN_BTN_OUVERTURE_MANUELLE 2
#define PIN_BTN_FERMETURE_MANUELLE 6
#define PIN_BTN_MODE 3//mode auto/manuel
#define PIN_DTR_MODULE_GSM 4//pour le mode veille


#endif
