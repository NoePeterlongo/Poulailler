#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Arduino.h"

//options
#define DEBUG_SERIAL true
#define TIMEOUT_MOTEUR_PORTE 5000 //timeout de securite = tps maximal d'activation du moteur (millisecondes)


//branchements
#define PIN_COMMANDE_MOTEUR_OUVERTURE 2
#define PIN_COMMANDE_MOTEUR_FERMETURE 3
#define PIN_CAPTEUR_PORTE_OUVERTE 4
#define PIN_CAPTEUR_PORTE_FERMEE 5



#endif