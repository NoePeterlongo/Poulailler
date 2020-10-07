
#include "configuration.h"
#include "gestionMoteur.h"
#include "calculSoleil.h"

void setup()
{
    if(DEBUG_SERIAL) { Serial.begin(9600); Serial.println("Initialisation du programme..."); }

    gestionMoteur::initialiser();

    int8_t lH, lM, cH, cM;

    calculSoleil(30,10, 2020, 45.9f, 6.15f, &lH, &lM, &cH, &cM);
    Serial.print(lH);Serial.print("h");Serial.println(lM);
    Serial.print(cH);Serial.print("h");Serial.println(cM);
    
}

void loop()
{
    
}