#include "configuration.h"

namespace gestionMoteur
{
    
    void initialiser();
    /* Les procedures d'ouverture et fermeture verifient l'etat de la porte
    via les butees, avant d'activer les moteurs.
    Elles peuvent verifier le courant
    Elles verifient l'etat de la porte apres. Le booleen de sortie vaut false s'il y a une erreur*/
    bool ouvrir();
    bool fermer();

    //Procedures manuelles
    void forcerOuverture();
    void forcerFermeture();
    void stop();

    //etats possibles
    enum enumEtatPorte{PORTE_OUVERTE, PORTE_FERMEE, INTERMEDIAIRE, ERREUR};
    //info
    enumEtatPorte etatPorte();
}
