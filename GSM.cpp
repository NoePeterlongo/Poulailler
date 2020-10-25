#include "GSM.h"

namespace GSM
{
    CommandeGSM commande;
    SoftwareSerial *SIM800;
    bool commandeDisponible = false;
    int DTR;
    bool enVeille = false;

    void updateSerial()
    {
        delay(100);
        while(SIM800->available()) 
        {
            Serial.write(SIM800->read());//Forward what Software Serial received to Serial Port
        }
    }

    void init(int TX, int RX, int _DTR, String codePIN)
    {
        SIM800 = new SoftwareSerial(TX, RX);

        DTR = _DTR;
        if(DTR != -1)
        {
            pinMode(DTR, OUTPUT);
            digitalWrite(DTR, LOW);
            //Sortie de veille, si jamais
            SIM800->begin(9600);
            SIM800->print("AT\r");
            SIM800->print("AT+CSCLK=0\r");
            updateSerial();
        }

        SIM800->begin(115200);
        delay(100);
        SIM800->print("AT+IPR=9600\r");//Changement de baudrate, ca marche mieux a 9600 on dirait
        delay(100);
        SIM800->begin(9600);

        Serial.println("Verification de communication avec le module");
        SIM800->print("AT\r");
        updateSerial();
        Serial.println("Verification de presence de carte");
        SIM800->print("AT+CCID\r");
        updateSerial();

        if(!codePIN.equals(""))
            SIM800->print("AT+CPIN=\""+codePIN+"\"\r");
        updateSerial();

        // Set module to send SMS data to serial out upon receipt 
        SIM800->print("AT+CNMI=2,2,0,0,0\r");
        delay(100);
    }

    void MiseAJour()
    {
        if (SIM800->available() > 0)
        {
            if(enVeille)
            {
                digitalWrite(DTR, LOW);
                SIM800->print("AT\r");
                SIM800->print("AT+CSCLK=0\r");
                enVeille = false;
            }

            String ligne = SIM800->readStringUntil('\n');
            Serial.println(ligne);

            
            
            if (ligne.substring(0, 4).equals("+CMT"))//Reception de SMS
            {
                String numeroTel = ligne.substring(7, 19);
                ligne = SIM800->readStringUntil('\n');
                if(DEBUG_SERIAL) {Serial.print("SMS recu de "); Serial.print(numeroTel); Serial.print(" : "); Serial.println(ligne);}
                TraiterSMS(numeroTel, ligne);
            }
            //else Serial.println(ligne);
        }
    }

    void MiseEnVeille()
    {
        if(DTR != -1)
        {
            digitalWrite(DTR, HIGH);
            delay(100);
            SIM800->print("AT+CSCLK=1\r");
            delay(200);
            updateSerial();
        }
        else
            Serial.println("Erreur: pin DTR non definie");
        enVeille = true;
    }
    

    void EnvoyerSMS(String numero, String message) {
        // AT command to set SIM800 to SMS mode
        
        SIM800->print("AT+CMGF=1\r");
        delay(100);

        // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
        SIM800->println(String("AT+CMGS=\"" + numero + "\""));
        delay(100);

        // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
        SIM800->print(message);
        delay(100);
        // End AT command with a ^Z, ASCII code 26
        SIM800->println((char)26);
        delay(100);
        SIM800->println();
    }

    void TraiterSMS(String numero, String contenu)
    {
        commandeDisponible = true;
        commande.numeroEmetteur = numero;

        if (contenu.substring(0, 4).equalsIgnoreCase("help") || contenu.substring(0, 4).equalsIgnoreCase("aide") || contenu.substring(0, 3).equalsIgnoreCase("man"))
        {
            commande.typeCommande = typeCommande::aide;
            Serial.println("Commande type aide recue");
        }
        else if (contenu.substring(0, 4).equalsIgnoreCase("led ") && (contenu.charAt(4)=='1' || contenu.charAt(4) == '0'))
        {
            commande.typeCommande = typeCommande::commandeAParametre;
            commande.parametreBool = contenu.substring(4, 5).toInt();
        }
        else
            commande.typeCommande = typeCommande::nonValide;
    }

    CommandeGSM LireCommande()
    {
        if (commandeDisponible)
        {
            commandeDisponible = false;
            return commande;
        }
        else
        {
            Serial.println("Erreur : aucune commande disponible");
            commande.typeCommande = typeCommande::nonValide;//Code erreur
            return commande;
        }
    }
}
