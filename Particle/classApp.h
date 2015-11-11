/**
 * Copyright (c) 2015 par Marc Sibert.
 *
 * Project: Garden-V2
 * File name: app.h
 * Description:  Fichier d'entête de la classe App
 *   
 * @author Marc Sibert
 * @email marc@sibert.fr  
 *   
 * @see The GNU Public License (GPL)
 */
 
#pragma once

#ifndef __APP_H__
#define __APP_H__
 
#include <application.h>
#include "HttpClient/HttpClient.h"

/**
 * Classe générale associée à l'application. C'est un singleton qui ne peut être instancié qu'une seule fois.
 * Elle expose principalement deux méthodes setup et loop pour s'interfacer avec le mécanisme de lancement de la plateforme.
 */
class App {

public:
/**
 * Constructeur de l'application.
 * @param aAddr Adresse du composant BH1750 sur le bus i2c.
 */
    App(const bool aAddr = false);
 
/**
 * Méthode de lancement de l'application.
 * Cette méthode ne doit être appelée qu'une seule fois.
 */
    void setup();
    
/**
 * Méthode de traitement de l'application.
 * C'est une méthode itérative qui s'exécute toutes les 250ms.
 */
    void loop();
    
    
    
protected:
/**
 * Calcule une nouvelle valeur de la consigne sur la base d'un cosinus.
 */
    void calculerConsigne();
    
/**
 * Assure une itération de l'asservissement de l'éclairage de la lampe en mesurant la luminosité ambiante.
 */
    void regulerLampe();
    
/**
 * Envoie une trame http POST /log au serveur central.
 */
    void envoyerLog();
    
/**
 * Déclenche une mesure des paramètres de température et humidité ambiante.
 * Cette méthode met à jour les attributs fHumidity et fTemperature.
 */
    void mesurerMeteo();

/**
 * Déclenche une mesure du AM2302.
 * @param aHumid une référence sur le résultat de la mesure de l'humidité.
 * @param aTemp une référence sur le résultat de la mesure de température.
 * @note Les deux variables précédentes ne sont pas modifiées en cas d'échec de la mesure.
 * @return 0 en cas de succès, -1 en cas de timout, -2 en cas de CRC erroné.
 */
    int readOnce(double& aHumid, double& aTemp);
    
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
/**
 * Méthode appelée par l'interruption de réception des données de l'AM2302.
 */
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

/// Pointeur vers le singleton.
//    static App *fpApp;

/// L'adresse du composant sur le Bus i2c.
    const BH1750_ADDR fAddr;

/// Dernière valeur de puissance de la lampe.
    double fLamp;
    
/// Dernière valeur de mesure de luminosité.
    double fLuminosity;
    
/// Dernière consigne de luminosité.
    float fConsigne;
    
/// Dernière mesure de l'humidité.
    double fHumidity;
    
/// Dernière mesure de température.
    double fTemperature;
    
/// Un client http.
    HttpClient http;
    
    volatile byte p;
    volatile unsigned humid;
    volatile int temp;
    volatile byte cs;
    

};

#endif

