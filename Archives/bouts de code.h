//Cas intermediaires, determines par la luminosite
        if( (flags.etatSoleil == CREPUSCULE && flags.etatPorte==gestionMoteur::PORTE_OUVERTE)   || 
            (flags.etatSoleil == AUBE       && flags.etatPorte==gestionMoteur::PORTE_FERMEE)    )
        {
            //2 cas : soit on attend une confirmation de luminosite, soit non
            if(flags.enAttenteVerificationLuminosite)
            {
                if(millis()>datePremiereMesureLuminosite + DELAI_LUMINOSITE)//date de confirmation atteinte
                {
                    //confirmation soir
                    if(flags.etatSoleil == CREPUSCULE && analogRead(PIN_PHOTORESISTANCE) < SEUIL_VESPERAL)
                        gestionMoteur::fermer();

                    //confirmation matin
                    if(flags.etatSoleil == AUBE && analogRead(PIN_PHOTORESISTANCE) > SEUIL_MATINAL)
                        gestionMoteur::ouvrir();
                    
                    //Dans tous les cas on n'attend plus de confirmation
                    flags.enAttenteVerificationLuminosite = false;
                }
            }
            else if( (flags.etatSoleil == CREPUSCULE && analogRead(PIN_PHOTORESISTANCE) < SEUIL_VESPERAL) ||
                    (flags.etatSoleil == AUBE && analogRead(PIN_PHOTORESISTANCE) > SEUIL_MATINAL))
                {
                    flags.enAttenteVerificationLuminosite = true;
                    datePremiereMesureLuminosite = millis();
                }
        }
    
        //cas intermediaire V2
        if(flags.etatSoleil == CREPUSCULE && flags.etatPorte == gestionMoteur::PORTE_OUVERTE && analogRead(PIN_PHOTORESISTANCE) < SEUIL_VESPERAL)
        {
            if(flags.enAttenteVerificationLuminosite && millis() > datePremiereMesureLuminosite + DELAI_LUMINOSITE)
            {
                gestionMoteur::fermer();
                flags.enAttenteVerificationLuminosite = false;
            }
            if(!flags.enAttenteVerificationLuminosite)
            {
                flags.enAttenteVerificationLuminosite = true;
                datePremiereMesureLuminosite = millis();
            }
        }
        if(flags.etatSoleil == AUBE && flags.etatPorte == gestionMoteur::PORTE_FERMEE && analogRead(PIN_PHOTORESISTANCE) > SEUIL_MATINAL)
        {
            if(flags.enAttenteVerificationLuminosite && millis() > datePremiereMesureLuminosite + DELAI_LUMINOSITE)
            {
                gestionMoteur::ouvrir();
                flags.enAttenteVerificationLuminosite = false;
            }
            if(!flags.enAttenteVerificationLuminosite)
            {
                flags.enAttenteVerificationLuminosite = true;
                datePremiereMesureLuminosite = millis();
            }
        }
    