
#include "configuration.h"
#include "gestionMoteur.h"

void setup()
{
    if(DEBUG_SERIAL) { Serial.begin(9600); Serial.println("Initialisation du programme..."); }

    gestionMoteur::initialiser();
    
}

void loop()
{
    
}