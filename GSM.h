
#ifndef GSM_H
#define GSM_H

#include <SoftwareSerial.h>
#include "configuration.h"

namespace GSM
{

    //Definir ici les types de commandes possibles
    enum typeCommande {
        nonValide = -1,
        aide,
        passageModeManuel,
        passageModeAutomatique,
        ouvrirPorte,
        fermerPorte,
        envoyerRapport,
        commandeAParametre
    };

    //Une commande est constituee d'un type et d'un ou plusieurs parametres
    struct CommandeGSM {
        int typeCommande = -1;

        String numeroEmetteur;

        bool parametreBool;
        int parametreInt;
        String parametreString;
    };

    extern SoftwareSerial *SIM800;//port serie utilise pour le module GSM
    void TraiterSMS(String numero, String SMS);//ne pas utiliser, est appele mar MiseAJour
    extern CommandeGSM commande;//Ne pas interroger, est renvoyee par LireCommande()
    extern int DTR;


    //Fonctions appelables:

    void init(int TX_Pin_Cote_GSM, int RX_Pin_Cote_GSM, int _DTR = -1, String codePIN = "");
    //a appeler a chaque boucle, pour surveiller le port serie
    void MiseAJour();
    void MiseEnVeille();

    //Une commande a ete recue
    extern bool commandeDisponible;

    //Permet de recuperer la derniere commande recue
    CommandeGSM LireCommande();

    void EnvoyerSMS(String numero, String message);

}

#endif
