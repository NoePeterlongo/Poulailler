
#include "configuration.h"
#include "gestionMoteur.h"
#include "calculSoleil.h"
#include "DS3231.h"


//FLAGS
bool ouvertureManuelleEnCours = false, fermetureManuelleEnCours = false;

//Horloge
DS3231 Horloge;
bool h12, PM, century=false;//Necessaires mais non utilisees. Pas besoin d'initialisation (sauf century ?)
unsigned long dateDernierAffichage = 0;

void setup()
{
    delay(1000);
    if(DEBUG_SERIAL) { Serial.begin(9600); Serial.println("Initialisation du programme..."); }

    gestionMoteur::initialiser();

    pinMode(PIN_BTN_FERMETURE_MANUELLE, INPUT_PULLUP);
    pinMode(PIN_BTN_OUVERTURE_MANUELLE, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    
    bool ilFaitJour = jourSoleil(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute());
    
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

    //Affichage des mesures
    if(millis() > dateDernierAffichage + 2000)
    {
        dateDernierAffichage = millis();
        Serial.print(Horloge.getDate()); Serial.print(";");
        Serial.print(Horloge.getMonth(century)); Serial.print(";");
        Serial.print(2000+Horloge.getYear()); Serial.print(";");
        Serial.print(Horloge.getHour(h12, PM)); Serial.print(";");
        Serial.print(Horloge.getMinute()); Serial.print(";");
        Serial.print(Horloge.getSecond()); Serial.print(";");
        Serial.print(Horloge.getTemperature()); Serial.print(";");
        bool ilFaitJour = jourSoleil(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute());
        Serial.print(ilFaitJour); Serial.print(";");
        Serial.println(analogRead(A1)); 
    }
}