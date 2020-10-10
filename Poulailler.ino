
#include "configuration.h"
#include "gestionMoteur.h"
#include "calculSoleil.h"
#include "DS3231.h"


//FLAGS
bool ouvertureManuelleEnCours = false, fermetureManuelleEnCours = false;

//Horloge
DS3231 Horloge;
bool h12, PM, century=false;//Necessaires mais non utilisees. Pas besoin d'initialisation (sauf century ?)

void setup()
{
    delay(1000);
    if(DEBUG_SERIAL) { Serial.begin(9600); Serial.println("Initialisation du programme..."); }

    gestionMoteur::initialiser();

    pinMode(PIN_BTN_FERMETURE_MANUELLE, INPUT_PULLUP);
    pinMode(PIN_BTN_OUVERTURE_MANUELLE, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    
    bool ilFaitJour = jour(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute());
    
    //Aller-retour d'initialisation
    if(gestionMoteur::ouvrir()) Serial.println("Porte ouverte");
    delay(500);
    if(gestionMoteur::fermer()) Serial.println("Porte fermee");
}

void loop()
{
    //Ouverture/fermeture manuelle
    bool btnOuverture = digitalRead(PIN_BTN_OUVERTURE_MANUELLE) == BTN_ACTIF;
    bool btnFermeture = digitalRead(PIN_BTN_FERMETURE_MANUELLE) == BTN_ACTIF;
    if(btnOuverture && !ouvertureManuelleEnCours)
    {
        gestionMoteur::forcerOuverture();
        ouvertureManuelleEnCours = true;
    }
    if(!btnOuverture && ouvertureManuelleEnCours)
    {
        gestionMoteur::stop();
        ouvertureManuelleEnCours = false;
    }
    if(btnFermeture && ! fermetureManuelleEnCours)
    {
        gestionMoteur::forcerFermeture();
        fermetureManuelleEnCours = true;
    }
    if(!btnFermeture && fermetureManuelleEnCours)
    {
        gestionMoteur::stop();
        fermetureManuelleEnCours = false;
    }
}