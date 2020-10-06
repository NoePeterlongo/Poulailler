#include "gestionMoteur.h"

namespace gestionMoteur{

    void initialiser()
    {
        if(DEBUG_SERIAL) Serial.println("Initialisation des entrees/sorties relatives a la porte");
        pinMode(PIN_COMMANDE_MOTEUR_OUVERTURE, OUTPUT);
        pinMode(PIN_COMMANDE_MOTEUR_FERMETURE, OUTPUT);
        pinMode(PIN_CAPTEUR_PORTE_OUVERTE, INPUT);
        pinMode(PIN_CAPTEUR_PORTE_FERMEE, INPUT);
    }

    bool ouvrir()
    {
        if(DEBUG_SERIAL) Serial.println("Ouverture de la porte...");

        //securite : on met a 0 la sortie de fermeture
        digitalWrite(PIN_COMMANDE_MOTEUR_FERMETURE, LOW);

        //securite : on verifie que la porte n'est pas deja ouverte
        if(digitalRead(PIN_CAPTEUR_PORTE_OUVERTE))
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : la porte est deja ouverte");
            return false;
        }

        //delai pour l'electronique de puissance
        delay(500);

        //*****ouverture*****
        unsigned long dateDebutOuverture = millis();
        digitalWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, HIGH);

        //attente d'un evenement de fin
        while(millis() < dateDebutOuverture + TIMEOUT_MOTEUR_PORTE && !digitalRead(PIN_CAPTEUR_PORTE_OUVERTE)); 

        digitalWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, LOW);
        
        //Erreur si la porte n'est pas ouverte
        if(!digitalRead(PIN_CAPTEUR_PORTE_OUVERTE))
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : timeout atteint, la porte n'est pas ouverte");
            return false;
        }
        //sinon tout est OK
        return true;
    }

    bool fermer()
    {
        if(DEBUG_SERIAL) Serial.println("Fermeture de la porte...");

        //securite : on met a 0 la sortie d'ouverture
        digitalWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, LOW);

        //securite : on verifie que la porte n'est pas deja fermee
        if(digitalRead(PIN_CAPTEUR_PORTE_FERMEE))
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : la porte est deja fermee");
            return false;
        }

        //delai pour l'electronique de puissance
        delay(500);

        //*****fermeture*****
        unsigned long dateDebutFermeture = millis();
        digitalWrite(PIN_COMMANDE_MOTEUR_FERMETURE, HIGH);

        //attente d'un evenement de fin
        while(millis() < dateDebutFermeture + TIMEOUT_MOTEUR_PORTE && !digitalRead(PIN_CAPTEUR_PORTE_FERMEE)); 

        digitalWrite(PIN_COMMANDE_MOTEUR_FERMETURE, LOW);
        
        //Erreur si la porte n'est pas fermee
        if(!digitalRead(PIN_CAPTEUR_PORTE_FERMEE))
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : timeout atteint, la porte n'est pas fermee");
            return false;
        }
        //sinon tout est OK
        return true;
    }


    void forcerOuverture()
    {
        //securite
        digitalWrite(PIN_COMMANDE_MOTEUR_FERMETURE, LOW);
        delay(100);

        digitalWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, HIGH);
    }

    void forcerFermeture()
    {
        //securite
        digitalWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, LOW);
        delay(100);

        digitalWrite(PIN_COMMANDE_MOTEUR_FERMETURE, HIGH);
    }

    void stop()
    {
        digitalWrite(PIN_COMMANDE_MOTEUR_FERMETURE, LOW);
        digitalWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, LOW);
    }

    int etatPorte()
    {
        bool capteurOuverture = digitalRead(PIN_CAPTEUR_PORTE_OUVERTE);
        bool capteurFermeture = digitalRead(PIN_CAPTEUR_PORTE_FERMEE);

        if(capteurFermeture && ! capteurOuverture)
            return porteFermee;
        if(capteurOuverture && ! capteurFermeture)
            return porteOuverte;
        if(!capteurFermeture && ! capteurOuverture)
            return intermediaire;
        if(capteurOuverture && capteurFermeture)
            return erreur;
    }
}
