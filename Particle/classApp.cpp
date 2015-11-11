/**
 * Copyright (c) 2015 par Marc Sibert.
 *
 * Project: Garden-V2
 * File name: app.cpp
 * Description:  Fichier de définition de la classe App
 *   
 * @author Marc Sibert
 * @email marc@sibert.fr  
 *   
 * @see The GNU Public License (GPL)
 */
 
#include "classApp.h"
#include <math.h>


/*
volatile byte App::p;
volatile unsigned App::humid;
volatile int App::temp;
volatile byte App::cs;
*/

void App::calculerConsigne() {
// recalcul de la consigne
    const float hms = Time.hour() + Time.minute() / 60.0;

    static const float S = 6;
    static const float D = 14;
    static const float A = 1000;
    
    if ((hms < S) || ( hms > S + D)) {
        fConsigne = 0;
    } else {
        fConsigne = A * (1 - cos((2.0 * M_PI) * (hms - S) / D)) / 2.0;
    }
}

void App::regulerLampe() {
    static float i = 0; // from P{I}D
    
    const byte nb = Wire.requestFrom(fAddr, 2);
    if (nb != 2) {
        Serial.print("Erreur ");
        Serial.print(nb);
        Serial.println(" byte, 2 attendus");
    }
    fLuminosity = (Wire.read() * 256 + Wire.read()) / 1.2 / 2;

    const float p = (fConsigne - 0 * fLuminosity) * 0.8;
    const float d = 0;
    const float di = (fConsigne - fLuminosity) / 200.0;
    if (((p + i + d <= 255) && (di > 0)) || ((p + i + d >= 0) && (di < 0))) {
        i += di;
    }
    
    const float l = constrain(p + i + d, 0, 255);

    if (round(l) != round(fLamp * 2.55)) analogWrite(D3, round(l));   // pour éviter le syntillement.
    fLamp = l / 2.55;
}

void App::envoyerLog() {
    http_header_t headers[] = {
        { "Content-Type", "application/json" },
        //  { "Accept" , "application/json" },
        { "Accept" , "*/*"},
        { NULL, NULL } // NOTE: Always terminate headers will NULL
    };

    String body("{ \"timestamp\": \"");
    body.concat(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
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
    
    http_request_t request = {
        "garden.dispositifs.fr",    // hostname
        IPAddress((uint8_t*)NULL),  // ip
        "/api/log",                 // path
        80,                         // port
        body                        // body
    };

    http_response_t response;
    
    http.post(request, response, headers);    

    if (response.status != 200) {
        String err;
        err.concat("Error POST /log ");
        err.concat(response.status);
        err.concat("\n");
        err.concat(response.body);
        Particle.publish("garden/error", err, 60, PRIVATE);
    }
}

void App::mesurerMeteo() {
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

int App::readOnce(double& aHumid, double& aTemp) {
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

App::App(const bool aAddr) : 
    fAddr(aAddr ? BH1750_I2CADDR_HIGH : BH1750_I2CADDR_LOW) {
}

void App::setup() {
//        waitUntil(WiFi.ready);
//        waitUntil(Particle.connected);

    pinMode(D3, OUTPUT);

    Wire.begin();
    Wire.beginTransmission(fAddr);
    Wire.write(BH1750_MODE_CONTINUOUSHIGHRES2);
    const byte err = Wire.endTransmission();
    if (err) {
        Particle.publish("garden/error", String(err), 60, PRIVATE);
    }
};

void App::loop() {
    unsigned long t;
    while ((t = millis()) % 250) delay(1);

    switch ((t / 250) % 4) { // 4 time slots de 250 ms
        case 0:
            calculerConsigne();
            regulerLampe();
            
            if (!(t % 5000)) {  // une fois toutes les 5 secondes seulement
                envoyerLog();
            }
            break;
        case 1:
            calculerConsigne();
            regulerLampe();
            break;
        case 2:
            calculerConsigne();
            regulerLampe();
            if ((t % 2000) == 500) {  // une lecture toutes les 2 secondes seulement
                mesurerMeteo();
            }            
            break;
        case 3:
            calculerConsigne();
            regulerLampe();
            break;
    }
};

