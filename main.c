#include "weather.h"

int main() 
{
    visa_vader();
    int val = valjstad();       // Låter användaren välja stad
    ta_vader(val);              //anropa funktionen från weather.c
           
    return 0;
}
