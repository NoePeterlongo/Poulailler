/* POULAILLER 03 v03 (issu de test23d6) FONCTIONNEL
 *  
 *  test23d6 : commande moteur PWM par pont en H 43A (BTS7960b de 2016)
    AVEC mesure de courant pour détection moteur en butée
      (moteur lève-vitre avec ressort : environ 2A à vide et 20A (+ ?) en butée)
    AVEC utilisation des inters fin de course ;
    AVEC utilisation de FLAGs "ouvert" et "fermé" pour ne plus commander le moteur  même si la commande
            d'ouverture ou de fermeture persiste
    OK pour démarrage par poussoir et arrêt par inter fin de course associé
    OK pour interdire le redémarrage [=inhiber l'ordre d'ouverture/fermeture] si fin_de_course activé ?????
    OK pour rester en mode Manuel tant qu'on ne rappuie pas sur le même poussoir
    AVEC procédures ouverture()et  fermeture() : OK
    AVEC LED éclairage commandée par TIP31C
    OK pour les temporisations avant ouverture, fermeture et extinction
 
 Circuit 7960 :
  masse logique et masse alim communes (?)
  valider le circuit en passant R_EN ou L_EN à 5V (puis repasser à 0 pour désactiver circuit et économie énergie)
  B+, B- vers batterie 12V
  M+, M- vers moteur
  RPWM et LPWM = entrées pour commander dans un sens ou l'autre.
Câblage module DS2332 : SDA sur A4 et SCL sur A5
 */
    // Déclarations : PAS dans le Setup       // INACTIF =1 car INPUT_PULLUP
boolean ouvrirM = false;   boolean fermerM = false;  // commandes MANUELLES (poussoirs) de la porte pour pont H (PWM)
boolean ouvrirA = false;   boolean fermerA = false;  // commandes AUTOMATIQUES (luminosité) de la porte pour pont H (PWM)
boolean ouvrir = false;   boolean fermer = false;  // commandes manuelles OU automatiques de la porte pour pont H (PWM)
int Butee1 = 1;   int Butee2 = 1;  // inters de fin de course ; ouverture associée à Butee1 et fermeture à Butee2
int pinLampe=9 ; // LED éclairage intérieure commandée via un TBip
const int poussoirO = 2; const int poussoirF = 6 ;  // boutons poussoirs Ouverture et Fermeture MANUELLES
const int vitesseM = 250 ;  const int vitesseA = 250 ;                 // vitesses Auto et Manuelle pour PWM (255 max)
const int Sortie1 = 10 ; const int Sortie2 = 5; // broches de sortie avec PWM possible
boolean ouvert=false, ferme=false;   // FLAG : la porte est à l'état "ouvert" / la porte est à l'état "fermé"
boolean attenteO=false, attenteF=false, attenteE=false ;    // FLAGs : attente d'ouverture, de fermeture, d'extinction de
                                                              // la lampe intérieure (pour gestion par tempos) 
boolean manuel=false;   // FLAG : le système fonctionne en mode "manuel" (forcé)
const int niveauLampeMax=5; int niveauLampe=niveauLampeMax;    // Led commandée à niveauLampeMax/255 en PWM
const int Hall = 2 ;  // broche 2 (A2) pour lecture courant
int courant = 511 ;  // valeur lue ; 511 : position neutre
const int courantMin = 350 ;  // valeur MIN à l'ouverture ; (350 si vitesse max=255) ; ATTENTION : dépend de vitesse
const int courantMax = 690 ;  // valeur MIN à l'ouverture ; (690 si vitesse max=255) ; ATTENTION : dépend de vitesse
const int photoR = 0 ;  // broche 0 (A0) pour lecture tension photo Résistance
int tension_photoR = 100 ;  // valeur lue ; la photoR fait 1 pont diviseur avec 100k sur 5V
const int seuil_diurne = 85 ; const int seuil_nocturne = 150 ; //seuils de luminosité, photorésistance en série avec 100k
                                              // seuils standards : 80 et 110
int tempo1, tempo2;
float x1=0.0000000000,x2=0.00000000,x3,x4;
unsigned long t, t1, t2, t3;     // pour tempo (en ms) ouverture/fermeture porte + extinction LED éclairage
unsigned long tempoF=1200000L, tempoO=6000L, tempoE=1300000L ; // "L" : forcés en "long" (32 bits); 20, 10 et 22min (en ms)
                // F : avant fermeture, O : avant ouverture, E : durée éclairage                       
//unsigned long tempoF=20000L, tempoO=10000L, tempoE=30000L ; // valeurs pour test seulement


void setup() {   //configure pins as an input and enable the internal pull-up resistor
  pinMode(poussoirO, INPUT_PULLUP); pinMode(poussoirF, INPUT_PULLUP); // inters poussoirs de commande : SENS 1 et 2
  pinMode(11, INPUT_PULLUP); pinMode(12, INPUT_PULLUP); // inters fin de course n°1 et 2
  pinMode(13, OUTPUT);  // LED intégrée sur Arduino Nano
  pinMode(10, OUTPUT);  pinMode(5,OUTPUT);  pinMode(pinLampe,OUTPUT);    // sorties pour PWM
  Serial.begin(9600);
}
void loop() {
  digitalWrite(13, LOW); digitalWrite(Sortie1, LOW); digitalWrite(Sortie2, LOW);//par défaut on ne fait rien;13=LED de l'Arduino
  ouvrirM = !digitalRead(poussoirO);   fermerM = !digitalRead(poussoirF);  //LECTURE des commandes Manuelles (actives à 0)
  Butee1 = digitalRead(11);  Butee2 = digitalRead(12);  // LECTURE des inters fins de course (actifs à 0)
  courant = analogRead (Hall); // lecture du courant
  t=millis(); // définition du temps pour les tempos ; cf test24 pour les dépassement de calibre unsigned long (OK avec soustraction)
  tension_photoR = analogRead(photoR); // !!!!!!!!!!!!!!!!! ne pas oublier de la lire !!!!!!!!!!!!!!!!!! 
 
/*  Serial.print("t :"); Serial.print(t); Serial.print("  t1 (O) : "); Serial.print(t1); Serial.print("  t2 (F) : "); Serial.print(t2);
   Serial.print("  t3 (E) : "); Serial.print(t3); Serial.print("  tension photoR : "); Serial.print(tension_photoR);  
   Serial.print("  niveau lampe : "); Serial.print(niveauLampe);Serial.println(); 
  delay(300) ;          */

  if (Butee1==0) {ouvert=true;} else {ouvert=false;}  //...else indispensable..........
  if (Butee2==0) {ferme=true;} else {ferme=false;}//....idem...........
  
  if (ouvrirM == true && ouvert==false) //ouverture manuelle
     {manuel=true; ouvertureM(); while(manuel==true) // on RESTE en mode manuel tant qu'on ne rappuie pas sur le même poussoir
                                     {manuel = digitalRead(poussoirO); delay(50);}} // délai anti-rebond  }}
  if (fermerM == true && ferme==false) //fermeture manuelle
     {manuel=true; fermetureM(); while(manuel==true) // on RESTE en mode manuel tant qu'on ne rappuie pas sur le même poussoir
                                     {manuel = digitalRead(poussoirF); delay(50);}} // délai anti-rebond  } }
  //if (eclairerM == true) // {eclairage()}// test éclairage manuel 
    // {analogWrite(pinLampe,niveauLampeMax); t3=millis();  attenteE=true; }     // allumage ET attente extinction

  if (tension_photoR <= seuil_diurne) {ouvrirA=true;}  // ordre d'ouverture AUTO (luminosité)              
  if (tension_photoR >= seuil_nocturne) {fermerA=true;}  // ordre de fermeture AUTO (luminosité)              
  if (ouvrirA == true && ouvert==false && attenteO==false) {t1=millis(); attenteO=true;} 
                                                    // ouverture automatique : déclenchement de la tempo (t1=heure initiale)
  if (fermerA == true && ferme==false && attenteF==false) {t2=millis(); t3=millis(); attenteF=true; attenteE=true; 
             analogWrite(pinLampe,niveauLampeMax);} // fermeture automatique : déclenchement de la tempo (t2=heure initiale)                    
  t=millis(); // (en principe c'est une redite)
  //////////////////////////////// gestion des tempos : ///////////////////////////////////
  if (attenteO==true && t>tempoO+t1) {attenteO=false; ouvertureA();  } // Ouverture porte
  if (attenteF==true && t>tempoF+t2) {attenteF=false; fermetureA();  } // Fermeture porte
  if (attenteE==true && t>t3+tempoE) {attenteE=false; analogWrite(pinLampe,0); niveauLampe=niveauLampeMax; } // extinction lampe
  if (attenteE==true && t>t3 && t<(t3+tempoE)) {niveauLampe=int(niveauLampeMax*(tempoE)/(t-t3)); analogWrite(pinLampe,niveauLampe);}
                                                                    // diminution progressive du niveau d'éclairage
  }
void ouvertureM()  // Ouverture Manuelle
  {while(Butee1==HIGH  && ouvert==false) // OUVRIR ; supprimé les protections en courant
      {Butee1 = digitalRead(11); courant = analogRead (Hall); // actualiser l'état du fin de course et valeur du courant
       if (Butee1==0) {ouvert=true; ouvrirM=false; digitalWrite(13, LOW); analogWrite(Sortie1, 0); return;} else {}
       digitalWrite(13, HIGH); analogWrite(Sortie1, vitesseM);} // Sortie1 (10) = PWM 1 + LED bleue
  }
void fermetureM()  // fermeture Manuelle 
  {while(Butee2==HIGH && ferme==false)  // FERMER
      {Butee2 = digitalRead(12);courant = analogRead (Hall); // actualiser l'état du fin de course et valeur du courant
      if (Butee2==0) {ferme=true;fermerM=false; digitalWrite(13, LOW); analogWrite(Sortie2, 0); return;} else {}
      digitalWrite(13, HIGH); analogWrite(Sortie2, vitesseM);} // Sortie2 (5) = PWM 2 + LED rouge
  }
void ouvertureA()  // Ouverture Automatique 
  {while(Butee1==HIGH  && ouvert==false) // OUVRIR ; supprimé les protections en courant
      {Butee1 = digitalRead(11); courant = analogRead (Hall); // actualiser l'état du fin de course et valeur du courant
       if (Butee1==0) {ouvert=true; ouvrirA=false; digitalWrite(13, LOW); analogWrite(Sortie1, 0); return;} else {}
       digitalWrite(13, HIGH); analogWrite(Sortie1, vitesseA);} // Sortie1 (10) = PWM 1 + LED bleue
  }
void fermetureA()  // fermeture Automatique 
  {while(Butee2==HIGH && ferme==false)  // FERMER
      {Butee2 = digitalRead(12);courant = analogRead (Hall); // actualiser l'état du fin de course et valeur du courant
      if (Butee2==0) {ferme=true;fermerA=false; digitalWrite(13, LOW); analogWrite(Sortie2, 0); return;} else {}
      digitalWrite(13, HIGH); analogWrite(Sortie2, vitesseA);} // Sortie2 (5) = PWM 2 + LED rouge
  }
