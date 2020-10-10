#include "configuration.h"

void calculSoleil(int D, int M, int Y, float lat, float longitude,
                    int *minuteLever, int *minuteCoucher)
    {
        //Rang du jour
        //Calcul jour julien https://fr.wikipedia.org/wiki/Jour_julien
        int M2 = M, Y2=Y;
        if(M2<=2)
        {
            Y2 -= 1;
            M2 += 12;
        }
        int S = (int)(Y2/100);
        int B = 2 - S + (int)(S/4);
        double JDN = (long) (365.25*Y2) + (long) (30.6001*(M2+1)) + D + B + 1720994.5;

        double n = JDN - 0.5 - 2458848 - (int)((Y-2020)*365.25);

        bool leapYear = (Y % 4 == 0);
        if(!leapYear) n -= 1;
        //n est le jour de l'annee

        double pi = 3.1415926536;
        float deg2rad = pi/180, rad2deg = 180/pi;

        //Fractionnal year fY
        double fY = (n-1)*2*pi/(leapYear ? 366 : 365);

        //Equation of time
        double eqtime = 229.18*(0.000075 + 
                                0.001868*cos(fY) - 
                                0.032077*sin(fY) -
                                0.014615*cos(2*fY) - 
                                0.040849*sin(2*fY));
        
        //solar declinaison angle
        double decl =   0.006918 - 
                        0.399912 * cos(fY) + 
                        0.070257 * sin(fY) - 
                        0.006758 * cos(2*fY) +
                        0.000907 * sin(2*fY) -
                        0.002697 * cos(3*fY) + 
                        0.00148  * sin(3*fY);

        //Hour angle
        double ha = acos((cos(90.833*deg2rad)/(cos(lat*deg2rad)*cos(decl))) - tan(lat*deg2rad)*tan(decl));

        //heures en minutes en utc 
        double sunrise = 720 - 4*(longitude + ha*rad2deg) - eqtime;
        double sunset = 720 - 4*(longitude - ha*rad2deg) - eqtime;

        /*Serial.print((int)(sunrise/60)); Serial.print("h"); Serial.print((int)sunrise%60);
        Serial.print("\t");
        Serial.print((int)(sunset/60));  Serial.print("h"); Serial.println((int)sunset%60);*/

        //Sortie
        *minuteLever = (int)sunrise;
        *minuteCoucher = (int)sunset;
    }

bool jour(int D, int M, int Y, int H, int Min, float offsetLeverMinutes=0, float offsetCoucherMinutes=0,
        float lat=45.9f, float longitude=6.15f)
{
    int minuteLever, minuteCoucher;
    calculSoleil(D, M, Y, lat, longitude, &minuteLever, &minuteCoucher);

    minuteLever += offsetLeverMinutes;
    minuteCoucher += offsetCoucherMinutes;

    int minuteActuelle = H*60 + Min;
    Serial.println(minuteLever); Serial.println(minuteActuelle); Serial.println(minuteCoucher);

    return minuteActuelle >= minuteLever && minuteActuelle <= minuteCoucher;
}
