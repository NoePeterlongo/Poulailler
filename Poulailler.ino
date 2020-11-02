
#include "configuration.h"
#include "GSM.h"
#include "gestionMoteur.h"
#include "calculSoleil.h"
#include "DS3231.h"

//Modes 
enum EnumMode {
    MANUEL, AUTOMATIQUE_NORMAL, AUTOMATIQUE_SANS_HORLOGE, AUTOMATIQUE_SANS_PHOTORESISTANCE
} ;

//FLAGS
struct {
    bool ouvertureManuelleEnCours = false, fermetureManuelleEnCours = false;
    bool enAttenteVerificationLuminosite = false, enAttenteVerificationLuminositeErreur = false;
    gestionMoteur::EnumEtatPorte etatPorte;
    EnumEtatSoleil etatSoleil; 
    bool btnModeEnfonce = false;//"le btn etait-il enfonce aux dernieres nouvelles ?"
    EnumMode mode = AUTOMATIQUE_NORMAL;
} flags;

unsigned long datePremiereMesureLuminosite, dateDerniereMesureLuminosite;
unsigned long datePremiereMesureLuminositeErreur, dateDerniereMesureLuminositeErreur;
double moyenneLuminosite, moyenneLuminositeErreur;
unsigned long nbPointsMoyenne = 0, nbPointsMoyenneErreur = 0;

//Horloge
DS3231 Horloge;
bool h12, PM, century=false;//Necessaires mais non utilisees. Pas besoin d'initialisation (sauf century ?)
unsigned long dateDernierAffichage = 0;

void setup()
{
    delay(500);
    if(DEBUG_SERIAL) { Serial.begin(9600); Serial.println("Initialisation du programme..."); }

    gestionMoteur::initialiser();

    pinMode(PIN_BTN_FERMETURE_MANUELLE, INPUT_PULLUP);
    pinMode(PIN_BTN_OUVERTURE_MANUELLE, INPUT_PULLUP);
    pinMode(PIN_BTN_MODE, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    //initialisation des flags
    flags.etatPorte = gestionMoteur::etatPorte();
    flags.etatSoleil = getEtatSoleil(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute(), MARGE_PRELEVER, MARGE_POSTLEVER, MARGE_PRECOUCHER, MARGE_POSTCOUCHER);
}

void loop()
{
    //btn mode 
    if(digitalRead(PIN_BTN_MODE) == BTN_ACTIF) flags.btnModeEnfonce = true;
    else if(flags.btnModeEnfonce && digitalRead(PIN_BTN_MODE) != BTN_ACTIF)//btn mode relache
    {
        //on attend et on verifie, pour eviter les rebonds
        delay(200);
        if(digitalRead(PIN_BTN_MODE) != BTN_ACTIF)
        {
            flags.btnModeEnfonce = false;
            flags.mode = flags.mode == EnumMode::MANUEL ? EnumMode::AUTOMATIQUE_NORMAL : EnumMode::MANUEL;
            if(DEBUG_SERIAL) {Serial.print("Passage en mode "); if(flags.mode==EnumMode::MANUEL) Serial.println("manuel"); else Serial.println("automatique");}
        }
    }
    else flags.btnModeEnfonce = false;//au cas ou

    //Procedure
    switch (flags.mode)
    {
    case EnumMode::MANUEL:
        ModeManuel();
        break;
    case EnumMode::AUTOMATIQUE_NORMAL:
        ModeAutomatiqueNormal();
        break;
    case EnumMode::AUTOMATIQUE_SANS_HORLOGE:
        ModeAutomatiqueSansHorloge();
        break;
    case EnumMode::AUTOMATIQUE_SANS_PHOTORESISTANCE:
        ModeAutomatiqueSansPhotoresistance();
        break;
    default:
        break;
    }

    //Affichage des mesures
    if(millis() > dateDernierAffichage + 60000)
    {
        dateDernierAffichage = millis();
        Serial.print(Horloge.getDate()); Serial.print(";");
        Serial.print(Horloge.getMonth(century)); Serial.print(";");
        Serial.print(2000+Horloge.getYear()); Serial.print(";");
        Serial.print(Horloge.getHour(h12, PM)); Serial.print(";");
        Serial.print(Horloge.getMinute()); Serial.print(";");
        Serial.print(Horloge.getSecond()); Serial.print(";");
        Serial.print(Horloge.getTemperature()); Serial.print(";");
        Serial.print((double)analogRead(A0)); Serial.print(";");
        Serial.print(flags.etatSoleil); Serial.print(";");
        Serial.print(flags.mode); Serial.print(";");
        Serial.print(gestionMoteur::etatPorte()); Serial.print(";");
        Serial.print(flags.enAttenteVerificationLuminosite); Serial.print(";");
        Serial.println(moyenneLuminosite);
    }
}

void ModeAutomatiqueNormal()
{   
    digitalWrite(LED_BUILTIN, LOW);

    //Determination jour/nuit
    flags.etatSoleil = getEtatSoleil(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute(), MARGE_PRELEVER, MARGE_POSTLEVER, MARGE_PRECOUCHER, MARGE_POSTCOUCHER);
    flags.etatPorte = gestionMoteur::etatPorte();

    //detection erreur d'horloge
    if(flags.etatSoleil == EnumEtatSoleil::ERREUR)
    {
        flags.mode = EnumMode::AUTOMATIQUE_SANS_HORLOGE; //Pour passer en mode sans horloge
        flags.enAttenteVerificationLuminosite = false;
        if(DEBUG_SERIAL) Serial.println("Erreur d'horloge, passage en mode sans horloge");
        return;
    }
    
    //***********detection erreur de photoresistance********************
    /* Cette verification se fait pendant les phases "JOUR" et "NUIT"
    Si la luminosite passe le "SEUIL_ERREUR" correspondant, on passe en mode sans photoresistance.
    Avant de passer en mode degrade, on fait une moyenne sur 10min pour verifier*/
    //debut
    if((flags.etatSoleil == JOUR && analogRead(PIN_PHOTORESISTANCE) > SEUIL_ERREUR_JOUR && !flags.enAttenteVerificationLuminositeErreur) ||
        (flags.etatSoleil == NUIT && analogRead(PIN_PHOTORESISTANCE) < SEUIL_ERREUR_NUIT && !flags.enAttenteVerificationLuminositeErreur))
    {
        flags.enAttenteVerificationLuminositeErreur = true;
        moyenneLuminositeErreur = analogRead(PIN_PHOTORESISTANCE);
        nbPointsMoyenneErreur = 1;
        dateDerniereMesureLuminositeErreur = millis();
        datePremiereMesureLuminositeErreur = millis();
    }
    //En cours de moyenne erreur
    if(flags.enAttenteVerificationLuminositeErreur && millis() > dateDerniereMesureLuminositeErreur + PERIODE_MESURE_LUMINOSITE)
    {
        dateDerniereMesureLuminositeErreur = millis();
        moyenneLuminositeErreur = (moyenneLuminositeErreur*nbPointsMoyenneErreur + (double)analogRead(PIN_PHOTORESISTANCE))/(nbPointsMoyenneErreur+1);
        nbPointsMoyenneErreur++;
    }

    //Fin de moyenne erreur, et decision
    if(flags.enAttenteVerificationLuminositeErreur && millis() > datePremiereMesureLuminositeErreur + DELAI_LUMINOSITE_VERIFICATION_ERREUR)
    {
        flags.enAttenteVerificationLuminositeErreur = false;
        if((moyenneLuminositeErreur > SEUIL_ERREUR_JOUR && flags.etatSoleil == JOUR) ||
            (moyenneLuminositeErreur < SEUIL_ERREUR_NUIT && flags.etatSoleil == NUIT))
        {
            if(DEBUG_SERIAL) 
            {
                Serial.print("Erreur de luminosite detectee, passage en mode sans photoresistance\nLuminosite mesuree : ");
                Serial.println(moyenneLuminositeErreur);
            }
            flags.mode = EnumMode::AUTOMATIQUE_SANS_PHOTORESISTANCE;
        }
    }

    //Fin si on passe a l'AUBE ou au CREPUSCULE
    if((flags.etatSoleil == AUBE || flags.etatSoleil == CREPUSCULE) && flags.enAttenteVerificationLuminositeErreur)
        flags.enAttenteVerificationLuminositeErreur = false;

    ///******************Fonctionnement normal********************

    //Cas determines par l'horloge
    if(flags.etatSoleil == JOUR && flags.etatPorte != gestionMoteur::PORTE_OUVERTE)
    {
        if(DEBUG_SERIAL) Serial.println("Ouverture forcee par RTC");
        gestionMoteur::ouvrir();
        flags.enAttenteVerificationLuminosite = false;
    }
    if(flags.etatSoleil == NUIT && flags.etatPorte != gestionMoteur::PORTE_FERMEE)
    {
        if(DEBUG_SERIAL) Serial.println("Fermeture forcee par RTC");
        gestionMoteur::fermer();
        flags.enAttenteVerificationLuminosite = false;
    }
    
    //Cas intermediaire, decide par luminosite
    if( (flags.etatSoleil == CREPUSCULE && flags.etatPorte!=gestionMoteur::PORTE_FERMEE)   || 
        (flags.etatSoleil == AUBE       && flags.etatPorte!=gestionMoteur::PORTE_OUVERTE)    )
    {
        //2 cas : soit on attend une confirmation de luminosite, soit non
        if(flags.enAttenteVerificationLuminosite)
        {

            if(millis()>datePremiereMesureLuminosite + DELAI_LUMINOSITE)//date de confirmation atteinte, fin de la moyenne
            {
                //confirmation soir
                if(flags.etatSoleil == CREPUSCULE && moyenneLuminosite > SEUIL_VESPERAL)//la tension baisse avec la luminosite
                    gestionMoteur::fermer();

                //confirmation matin
                if(flags.etatSoleil == AUBE && moyenneLuminosite < SEUIL_MATINAL)
                    gestionMoteur::ouvrir();
                
                //Dans tous les cas on n'attend plus de confirmation
                flags.enAttenteVerificationLuminosite = false;
            }
            else //Sinon on continue de calculer la moyenne
            {
                if(millis() > dateDerniereMesureLuminosite + PERIODE_MESURE_LUMINOSITE)
                {
                    dateDerniereMesureLuminosite = millis();
                    moyenneLuminosite = (moyenneLuminosite * nbPointsMoyenne + (double)analogRead(PIN_PHOTORESISTANCE))/(nbPointsMoyenne+1);
                    nbPointsMoyenne++;
                }
            }
            
        }
        else if( (flags.etatSoleil == CREPUSCULE && analogRead(PIN_PHOTORESISTANCE) > SEUIL_VESPERAL) ||
                (flags.etatSoleil == AUBE && analogRead(PIN_PHOTORESISTANCE) < SEUIL_MATINAL))
            {//On lance le calcul de moyenne si la luminosite passe le seuil
                flags.enAttenteVerificationLuminosite = true;
                datePremiereMesureLuminosite = millis();
                dateDerniereMesureLuminosite = millis();
                nbPointsMoyenne = 0;
            }
    }
}

void ModeAutomatiqueSansHorloge()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);

    //Pour savoir si on est en mode jour ou nuit, on regarde l'état de la porte
    //2 cas : soit on attend une confirmation de luminosite, soit non
    flags.etatPorte = gestionMoteur::etatPorte();

    if(flags.enAttenteVerificationLuminosite)
    {

        if(millis()>datePremiereMesureLuminosite + DELAI_LUMINOSITE)//date de confirmation atteinte, fin de la moyenne
        {
            //confirmation soir
            if(flags.etatPorte != gestionMoteur::PORTE_FERMEE && moyenneLuminosite > SEUIL_VESPERAL)//la tension baisse avec la luminosite
                gestionMoteur::fermer();

            //confirmation matin
            if(flags.etatPorte != gestionMoteur::PORTE_OUVERTE && moyenneLuminosite < SEUIL_MATINAL)
                gestionMoteur::ouvrir();
            
            //Dans tous les cas on n'attend plus de confirmation
            flags.enAttenteVerificationLuminosite = false;
        }
        else //Sinon on continue de calculer la moyenne
        {
            if(millis() > dateDerniereMesureLuminosite + PERIODE_MESURE_LUMINOSITE)
            {
                dateDerniereMesureLuminosite = millis();
                moyenneLuminosite = (moyenneLuminosite * nbPointsMoyenne + (double)analogRead(PIN_PHOTORESISTANCE))/(nbPointsMoyenne+1);
                nbPointsMoyenne++;
            }
        }
        
    }
    else if( (flags.etatPorte != gestionMoteur::PORTE_FERMEE && analogRead(PIN_PHOTORESISTANCE) > SEUIL_VESPERAL) ||
            (flags.etatPorte != gestionMoteur::PORTE_OUVERTE && analogRead(PIN_PHOTORESISTANCE) < SEUIL_MATINAL))
    {//On lance le calcul de moyenne si la luminosite passe le seuil
        flags.enAttenteVerificationLuminosite = true;
        datePremiereMesureLuminosite = millis();
        dateDerniereMesureLuminosite = millis();
        nbPointsMoyenne = 0;
    }
}

void ModeAutomatiqueSansPhotoresistance()
{
    /*
    Dans ce mode on enleve la gestion de la photoresistance qui 
    si elle est defectueuse risque de faire fermer trop tot.
    On utilise les marges habituelles: 
        On ouvre quand on est dans l'état "JOUR", 
        et on referme quand on est dans l'état "NUIT"
    */
    digitalWrite(LED_BUILTIN, HIGH);
    delay(30);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);

    //Determination jour/nuit
    flags.etatSoleil = getEtatSoleil(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute(), MARGE_PRELEVER, MARGE_POSTLEVER, MARGE_PRECOUCHER, MARGE_POSTCOUCHER);
    flags.etatPorte = gestionMoteur::etatPorte();

    //On ne vérifie pas l'etat de l'horloge (parce que bon, je ne saurais pas trop quoi faire si elle lache aussi)

    if(flags.etatSoleil == JOUR && flags.etatPorte != gestionMoteur::PORTE_OUVERTE)
    {
        gestionMoteur::ouvrir();
    }

    if(flags.etatSoleil == NUIT && flags.etatPorte != gestionMoteur::PORTE_FERMEE)
    {
        gestionMoteur::fermer();
    }
}

void ModeManuel()
{
    digitalWrite(LED_BUILTIN, HIGH);
    flags.enAttenteVerificationLuminosite = false;

    //Ouverture/fermeture manuelle
    bool btnOuverture = digitalRead(PIN_BTN_OUVERTURE_MANUELLE) == BTN_ACTIF;
    bool btnFermeture = digitalRead(PIN_BTN_FERMETURE_MANUELLE) == BTN_ACTIF;
    if(btnOuverture && !flags.ouvertureManuelleEnCours)
    {
        gestionMoteur::forcerOuverture();
        flags.ouvertureManuelleEnCours = true;
    }
    if(!btnOuverture && flags.ouvertureManuelleEnCours)
    {
        gestionMoteur::stop();
        flags.ouvertureManuelleEnCours = false;
    }
    if(btnFermeture && ! flags.fermetureManuelleEnCours)
    {
        gestionMoteur::forcerFermeture();
        flags.fermetureManuelleEnCours = true;
    }
    if(!btnFermeture && flags.fermetureManuelleEnCours)
    {
        gestionMoteur::stop();
        flags.fermetureManuelleEnCours = false;
    }
}
