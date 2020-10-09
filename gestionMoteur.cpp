#include "gestionMoteur.h"

namespace gestionMoteur{

    void initialiser()
    {
        if(DEBUG_SERIAL) Serial.println("Initialisation des entrees/sorties relatives a la porte");
        pinMode(PIN_COMMANDE_MOTEUR_OUVERTURE, OUTPUT);
        pinMode(PIN_COMMANDE_MOTEUR_FERMETURE, OUTPUT);
        pinMode(PIN_CAPTEUR_PORTE_OUVERTE, INPUT_PULLUP);
        pinMode(PIN_CAPTEUR_PORTE_FERMEE, INPUT_PULLUP);
    }

    bool ouvrir()
    {
        if(DEBUG_SERIAL) Serial.println("Ouverture de la porte...");

        //securite : on met a 0 la sortie de fermeture
        analogWrite(PIN_COMMANDE_MOTEUR_FERMETURE, 0);

        //securite : on verifie que la porte n'est pas deja ouverte
        if(digitalRead(PIN_CAPTEUR_PORTE_OUVERTE) == CAPTEUR_ACTIF)
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : la porte est deja ouverte");
            return false;
        }

        //delai pour l'electronique de puissance
        delay(500);

        //*****ouverture*****
        unsigned long dateDebutOuverture = millis();
        analogWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, VITESSE_MOTEUR);

        //attente d'un evenement de fin
        #if UTILISER_MESURE_COURANT
            while(millis() < dateDebutOuverture + TIMEOUT_MOTEUR_PORTE && 
                    digitalRead(PIN_CAPTEUR_PORTE_OUVERTE) != CAPTEUR_ACTIF &&
                    analogRead(PIN_CAPTEUR_COURANT_MOTEUR) < COURANT_MAX
                    );
        #else
            while(millis() < dateDebutOuverture + TIMEOUT_MOTEUR_PORTE && digitalRead(PIN_CAPTEUR_PORTE_OUVERTE) != CAPTEUR_ACTIF); 
        #endif

        analogWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, 0);
        
        //Erreur si la porte n'est pas ouverte
        if(digitalRead(PIN_CAPTEUR_PORTE_OUVERTE) != CAPTEUR_ACTIF)
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
        analogWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, 0);

        //securite : on verifie que la porte n'est pas deja fermee
        if(digitalRead(PIN_CAPTEUR_PORTE_FERMEE) == CAPTEUR_ACTIF)
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : la porte est deja fermee");
            return false;
        }

        //delai pour l'electronique de puissance
        delay(500);

        //*****fermeture*****
        unsigned long dateDebutFermeture = millis();
        analogWrite(PIN_COMMANDE_MOTEUR_FERMETURE, VITESSE_MOTEUR);

        //attente d'un evenement de fin
        #if UTILISER_MESURE_COURANT
            while(millis() < dateDebutFermeture + TIMEOUT_MOTEUR_PORTE && 
                    digitalRead(PIN_CAPTEUR_PORTE_FERMEE) != CAPTEUR_ACTIF &&
                    analogRead(PIN_CAPTEUR_COURANT_MOTEUR) < COURANT_MAX
                    );
        #else
            while(millis() < dateDebutFermeture + TIMEOUT_MOTEUR_PORTE && digitalRead(PIN_CAPTEUR_PORTE_FERMEE) != CAPTEUR_ACTIF); 
        #endif

        analogWrite(PIN_COMMANDE_MOTEUR_FERMETURE, 0);
        
        //Erreur si la porte n'est pas fermee
        if(digitalRead(PIN_CAPTEUR_PORTE_FERMEE) != CAPTEUR_ACTIF)
        {
            if(DEBUG_SERIAL) Serial.println("Erreur : timeout atteint, la porte n'est pas fermee");
            return false;
        }
        //sinon tout est OK
        return true;
    }


    void forcerOuverture()
    {
        if(DEBUG_SERIAL) Serial.println("Ouverture forcee");
        //securite
        analogWrite(PIN_COMMANDE_MOTEUR_FERMETURE, 0);
        delay(100);

        analogWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, VITESSE_MOTEUR);
    }

    void forcerFermeture()
    {
        if(DEBUG_SERIAL) Serial.println("Fermeture forcee");
        //securite
        analogWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, 0);
        delay(100);

        analogWrite(PIN_COMMANDE_MOTEUR_FERMETURE, VITESSE_MOTEUR);
    }

    void stop()
    {
        if(DEBUG_SERIAL) Serial.println("Stop moteur");
        analogWrite(PIN_COMMANDE_MOTEUR_FERMETURE, 0);
        analogWrite(PIN_COMMANDE_MOTEUR_OUVERTURE, 0);
    }

    int etatPorte()
    {
        bool capteurOuverture = digitalRead(PIN_CAPTEUR_PORTE_OUVERTE) == CAPTEUR_ACTIF;
        bool capteurFermeture = digitalRead(PIN_CAPTEUR_PORTE_FERMEE) == CAPTEUR_ACTIF;

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
