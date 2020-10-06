#include "configuration.h"

namespace gestionMoteur
{
    
    void initialiser();
    bool ouvrir();
    bool fermer();

    //Procedures manuelles
    void forcerOuverture();
    void forcerFermeture();
    void stop();

    //info
    int etatPorte();
    //etats possibles
    const int porteOuverte = 0, porteFermee = 1, erreur = -1, intermediaire = 2; 
}
