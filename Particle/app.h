/**
 * 
 **/
 
#pragma once

#ifndef __APP_H__
#define __APP_H__
 
#include <application.h>
#include "HttpClient/HttpClient.h"


/**
 * low precision cos function.
 * @see http://lab.polygonal.de/?p=205
 **/
inline float cos(float x) {
#define 	M_PI   3.14159265358979323846 /* pi */
#define 	M_PI_2   1.57079632679489661923 /* pi/2 */
#define 	M_PI_4   0.78539816339744830962 /* pi/4 */

//compute cosine: sin(x + PI/2) = cos(x)
    x += M_PI_2;

//always wrap input angle to -PI..PI
    while (x < -M_PI) x += 2 * M_PI;
    while (x >  M_PI) x -= 2 * M_PI;

    return (1.27323954 * x + (x < 0 ? 1 : -1) * 0.405284735 * x * x);
}


/**
 * Classe générale associée à l'application.
 * Elle expose principalement deux méthodes setup et loop pour s'interfacer avec le mécanisme de lancement de la plateforme.
 **/
class App {
protected:
/// Enumération des adresses du composant
    enum BH1750_ADDR {
        BH1750_I2CADDR_LOW = 0x23,
        BH1750_I2CADDR_HIGH = 0xED
    };

/// Enumération des fonctions du composant
    enum BH1750_MODES { 
//Start measurement at 1lx resolution. Measurement time is approx 120ms.
        BH1750_MODE_CONTINUOUSHIGHRES    = 0x10, 
//Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
        BH1750_MODE_CONTINUOUSHIGHRES2   = 0x11,
//Start measurement at 4lx resolution. Measurement time is approx 16ms.
        BH1750_MODE_CONTINUOUSLOWRES     = 0x13,
//Start measurement at 1lx resolution. Measurement time is approx 120ms.
//Device is automatically set to Power Down after measurement.
        BH1750_MODE_ONETIMEHIGHRES       = 0x20,
//Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
//Device is automatically set to Power Down after measurement.
        BH1750_MODE_ONETIMEHIGHRES2      = 0x21,
//Start measurement at 1lx resolution. Measurement time is approx 120ms.
//Device is automatically set to Power Down after measurement.
        BH1750_MODE_ONETIMELOWRES        = 0x23
    };

private:
/// L'adresse du composant sur le Bus i2c.
    const BH1750_ADDR fAddr;

/// Dernière valeur de puissance de la lampe.
    double fLamp;
    
/// Dernière valeur de mesure de luminosité.
    double fLuminosity;
    
/// Dernière consigne de luminosité.
    float fConsigne;
    
    double fHumidity;
    
    double fTemperature;
    
/// Un client http.
    HttpClient http;
    
private:    
    static volatile byte p;
    static volatile unsigned humid;
    static volatile int temp;
    static volatile byte cs;
    
    void interruptDataAM2302() {
//        noInterrupts();
        static unsigned long high = 0;
        static unsigned long last = 0;
    
        const unsigned long front = System.ticks();
        const unsigned d = front - last;
        last = front;
        if (p % 2) {    // impaire
            const unsigned long low = d;
            if ((p > 4) && (p < 36)) {
                humid = humid * 2 + (high > low ? 1 : 0);
            } else if ((p > 36) && (p < 68)) {
                temp = temp * 2 + (high > low ? 1 : 0);
            } else if ((p > 68) && (p < 84)) {
                cs = cs * 2 + (high > low ? 1 : 0);
            }
        } else {    // paire
            high = d;
        }
        ++p;
//        interrupts();
    }

protected:

/**
 * Assure une itération de l'asservissement de l'éclairage de la lampe en mesurant la luminosité ambiante.
 **/
    void regulerLampe() {
        static float i = 0; // from P{I}D
        
        const byte nb = Wire.requestFrom(fAddr, 2);
        if (nb != 2) {
            Serial.print("Erreur ");
            Serial.print(nb);
            Serial.println(" byte, 2 attendus");
        }
        fLuminosity = (Wire.read() * 256 + Wire.read()) / 1.2 / 2;

        const float p = 0;
        i = i + (fConsigne - fLuminosity) / 250.0;
        const float d = 0;
        
        const float l = constrain(p + i + d, 0, 100);
    
        if (round(l * 2.55) != round(fLamp * 2.55)) analogWrite(D3, round(l * 2.55));   // pour éviter le syntillement.
        fLamp = l;
    }
    
/**
 * Callback appelée par l'infrastructure en cas de modification de la consigne.
 **/
    int setConsigne(String command) {
        fConsigne = command.toInt();
        return 0;
    }
    
/**
 * Déclenche une mesure du AH2032
 * @param aHumid une référence sur le résultat de la mesure de l'humidité.
 * @param aTemp une référence sur le résultat de la mesure de température.
 * @note Les deux variables précédentes ne sont pas modifiées en cas d'échec de la mesure.
 * @return 0 en cas de succès, -1 en cas de timout, -2 en cas de CRC erroné.
 **/
    int readOnce(double& aHumid, double& aTemp) {
        p = 0;
        humid = 0;
        temp = 0;
        cs = 0;
        
    // input + 1k pullup
        digitalWrite(D5, HIGH);
    // input + pullup
        pinMode(D5, OUTPUT);
    // output High
        digitalWrite(D5, LOW);
        delay(2);   // > 1 ms
    // input + 1k pullup
        pinMode(D5, INPUT);
    
    // listen...
        attachInterrupt(D5, &App::interruptDataAM2302, this, CHANGE);
        for (byte i = 0; (i < 10) && (p < 84); ++i) {
            delayMicroseconds(1000);
        }
        detachInterrupt(D5);
        if (p < 84) {
            return -1;  // error timeout 10ms
        }
        
        if ((((humid & 0xff) + (humid / 0x100) + (temp & 0xff) + (temp / 0x100)) & 0xff) != cs) return -2; // error CRC
        aHumid = humid / 10.0;
        aTemp = temp / 10.0;
        return 0;   // valid result
    }

public:
/**
 * Constructeur de l'application.
 * @param aAddr Adresse du composant BH1750 sur le bus i2c.
 **/
    App(const bool aAddr = false) : 
        fAddr(aAddr ? BH1750_I2CADDR_HIGH : BH1750_I2CADDR_LOW) {
    }
 
 /**
  * Accesseur Set de la consigne.
  **/
    void consigne(const float& aConsigne) {
        fConsigne = aConsigne;
    }

/**
 * Accesseur Get de la consigne.
 **/
    float consigne() const {
        return fConsigne;
    }

/**
 * Méthode de lancement de l'application.
 * Cette méthode ne doit être appelée qu'une seule fois.
 **/
    void setup() {
        waitUntil(WiFi.ready);
        waitUntil(Particle.connected);

        pinMode(D3, OUTPUT);

        Wire.begin();
        Wire.beginTransmission(fAddr);
        Wire.write(BH1750_MODE_CONTINUOUSHIGHRES2);
        const byte err = Wire.endTransmission();
        if (err) {
            Particle.publish("garden/error", String(err), 60, PRIVATE);
        }
    };
  
  /**
   * Méthode de traitement de l'application.
   * C'est une méthode itérative qui s'exécute toutes les 250ms.
   **/
    void loop() {
        unsigned long t;
        while ((t = millis()) % 250) delay(1);

        switch ((t / 250) % 4) { // 4 time slots de 250 ms
            case 0:
                regulerLampe();
                
                if (!(t % 5000)) {  // une fois toutes les 5 secondes seulement
                    http_header_t headers[] = {
                        { "Content-Type", "application/json" },
                        //  { "Accept" , "application/json" },
                        { "Accept" , "*/*"},
                        { NULL, NULL } // NOTE: Always terminate headers will NULL
                    };

                    time_t time = Time.now();
                    Time.format(time, TIME_FORMAT_ISO8601_FULL);
                    
                    String body("{ \"timestamp\": \"");
                    body.concat(Time.format(time, TIME_FORMAT_ISO8601_FULL));
                    body.concat("\", \"from\": \"");
                    body.concat(System.deviceID());
                    body.concat("\", \"luminosity\": ");
                    body.concat(fLuminosity);
                    body.concat(", \"lamp\": ");
                    body.concat(fLamp);
                    body.concat(", \"consigne\": ");
                    body.concat(fConsigne);
                    body.concat(", \"humidity\": ");
                    body.concat(fHumidity);
                    body.concat(", \"temperature\": ");
                    body.concat(fTemperature);
                    body.concat("}");
                    
                    http_request_t request;
                    request.hostname = "garden.dispositifs.fr";
                    request.port = 80;
                    request.path = "/api/log";
                    request.body = body;
                
                    http_response_t response;
                    
                    http.post(request, response, headers);    
                    if (response.status != 200) {
                        Particle.publish("garden/error", String("Error POST /log ") + String(response.status) + String(response.body), 60, PRIVATE);
                    }
                }
                break;
            case 1:
                regulerLampe();
                break;
            case 2:
                regulerLampe();
                if ((t % 2000) == 500) {  // une lecture toutes les 2 secondes seulement
                    const int err = readOnce(fHumidity, fTemperature);
                    switch (err) {
                        case -1:
                            Particle.publish("garden/error", String("Timeout reading AH2302"), 60, PRIVATE);
                            break;
                        case -2:
                            Particle.publish("garden/error", String("Faulty CRC reading AH2302"), 60, PRIVATE);
                            break;
                    }
                }            
                break;
            case 3:
                regulerLampe();

                if ((t % 60000) == 750) {  // un calcul toutes les minutes seulement
                // recalcul de la consigne
                    const float hms = Time.hour() + Time.minute() / 60.0;
                    static const float S = 6;
                    static const float D = 14;
                    static const float A = 1500;
                    
                    if ((hms < S) || ( hms > S + D)) {
                        fConsigne = 0;
                    } else {
                        fConsigne = A * (1 - cos((2.0 * 3.141592654) * (hms - S) / D)) / 2.0;
                    }
                }
                
                break;
        }
    };
};

#endif

