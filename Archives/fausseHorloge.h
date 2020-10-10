
#include "configuration.h"

namespace fausseHorloge{

    int D, M, Y, H, Min;
    double S = 0;
    unsigned long ancienT = 0;

    void maj()
    {
        S += (double)(millis()-ancienT)/1000;
        ancienT = millis();
        if(S>=60){Min+=(int)(S/60); S -= 60*((int)(S/60)); }
        if(Min>=60) {Min -= 60; H++;}
        if(H>=24) {H -= 24; D++;}
        //Les jours commencent a 0 :P
        if(D >= 30 && (M==4 || M==6 || M==9 || M==11)) {D -= 30; M++;}
        if(D >= 31 && (M==1 || M==3 || M==5 || M==7 || M==8 || M==10 || M==12)) {D -= 31; M++;}
        if(D >= 28 && M==2) {D -= 28; M++;}
        if(M >= 13) {M=1; Y++;}
    }

}
