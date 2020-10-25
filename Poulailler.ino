
#include "configuration.h"
#include "GSM.h"
#include "gestionMoteur.h"
#include "calculSoleil.h"
#include "DS3231.h"
#include "clignottement.h"


//FLAGS
struct {
    bool ouvertureManuelleEnCours = false, fermetureManuelleEnCours = false;
    bool modeManuel = false;
    bool enAttenteVerificationLuminosite = false;
    gestionMoteur::enumEtatPorte etatPorte;
    enumEtatSoleil etatSoleil; 
    bool btnModeEnfonce = false;//"le btn etait-il enfonce aux dernieres nouvelles ?"
    bool erreurHorloge = false;
} flags;

unsigned long datePremiereMesureLuminosite, dateDerniereMesureLuminosite;
double moyenneLuminosite;
unsigned long nbPointsMoyenne = 0;

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
    
    if(flags.etatPorte != gestionMoteur::PORTE_OUVERTE) gestionMoteur::ouvrir();
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
            flags.modeManuel = !flags.modeManuel;
            if(DEBUG_SERIAL) {Serial.print("Passage en mode "); if(flags.modeManuel) Serial.println("manuel"); else Serial.println("automatique");}
        }
    }
    else flags.btnModeEnfonce = false;//au cas ou

    if(flags.modeManuel)
    {
        ModeManuel();
    }
    //Mode automatique
    else
    {
        if(!flags.erreurHorloge)
            ModeAutomatiqueNormal();
        else//mode degrade
            ModeAutomatiqueSansHorloge();
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
        Serial.print(flags.modeManuel); Serial.print(";");
        Serial.print(gestionMoteur::etatPorte()); Serial.print(";");
        Serial.print(flags.enAttenteVerificationLuminosite); Serial.print(";");
        Serial.print(moyenneLuminosite); 
        Serial.print(flags.erreurHorloge); Serial.print(";");

    }
}

void ModeAutomatiqueNormal()
{   
    digitalWrite(LED_BUILTIN, LOW);

    //Determination jour/nuit
    flags.etatSoleil = getEtatSoleil(Horloge.getDate(), Horloge.getMonth(century), 2000+Horloge.getYear(), Horloge.getHour(h12, PM), Horloge.getMinute(), MARGE_PRELEVER, MARGE_POSTLEVER, MARGE_PRECOUCHER, MARGE_POSTCOUCHER);
    flags.etatPorte = gestionMoteur::etatPorte();

    //detection erreur d'horloge
    if(flags.etatSoleil = enumEtatSoleil::ERREUR)
    {
        flags.erreurHorloge = true; //Pour passer en mode sans horloge
        flags.enAttenteVerificationLuminosite = false;
        clignottement(5);
        if(DEBUG_SERIAL) Serial.println("Erreur d'horloge, passage en mode sans horloge");
    }

    //Cas determines par l'horloge
    if(flags.etatSoleil == JOUR && flags.etatPorte != gestionMoteur::PORTE_OUVERTE)
    {
        if(DEBUG_SERIAL) Serial.println("Ouverture forcee par RTC");
        clignottement(0.5f);//Signalement de cette anomalie
        gestionMoteur::ouvrir();
        flags.enAttenteVerificationLuminosite = false;
    }
    if(flags.etatSoleil == NUIT && flags.etatPorte != gestionMoteur::PORTE_FERMEE)
    {
        if(DEBUG_SERIAL) Serial.println("Fermeture forcee par RTC");
        clignottement(1);//Signalement de cette anomalie
        gestionMoteur::fermer();
        flags.enAttenteVerificationLuminosite = false;
    }
    
    //Cas intermediaire, decide par luminosite
    if( (flags.etatSoleil == CREPUSCULE && flags.etatPorte==gestionMoteur::PORTE_OUVERTE)   || 
        (flags.etatSoleil == AUBE       && flags.etatPorte==gestionMoteur::PORTE_FERMEE)    )
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
    //Pour savoir si on est en mode jour ou nuit, on regarde l'état de la porte
    //2 cas : soit on attend une confirmation de luminosite, soit non
    flags.etatPorte = gestionMoteur::etatPorte();
    
    if(flags.enAttenteVerificationLuminosite)
    {

        if(millis()>datePremiereMesureLuminosite + DELAI_LUMINOSITE)//date de confirmation atteinte, fin de la moyenne
        {
            //confirmation soir
            if(flags.etatPorte == gestionMoteur::PORTE_OUVERTE && moyenneLuminosite > SEUIL_VESPERAL)//la tension baisse avec la luminosite
                gestionMoteur::fermer();

            //confirmation matin
            if(flags.etatPorte == gestionMoteur::PORTE_FERMEE && moyenneLuminosite < SEUIL_MATINAL)
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
    else if( (flags.etatPorte == gestionMoteur::PORTE_OUVERTE && analogRead(PIN_PHOTORESISTANCE) > SEUIL_VESPERAL) ||
            (flags.etatPorte == gestionMoteur::PORTE_FERMEE && analogRead(PIN_PHOTORESISTANCE) < SEUIL_MATINAL))
    {//On lance le calcul de moyenne si la luminosite passe le seuil
        flags.enAttenteVerificationLuminosite = true;
        datePremiereMesureLuminosite = millis();
        dateDerniereMesureLuminosite = millis();
        nbPointsMoyenne = 0;
    }
}

void ModeManuel()
{
    digitalWrite(LED_BUILTIN, HIGH);
    clignottementOff();

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
