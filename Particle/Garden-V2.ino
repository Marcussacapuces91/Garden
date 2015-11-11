/**
 * Copyright (c) 2015 par Marc Sibert.
 *
 * Project: Garden-V2
 * File name: garden-v2.ino
 * Description:  Fichier du projet Garden V2 pour le Photon/Particle
 *   
 * @author Marc Sibert
 * @email marc@sibert.fr  
 *   
 * @see The GNU Public License (GPL)
 */

// SYSTEM_THREAD(ENABLED);

#include "classApp.h"

App app;

void setup() {
    app.setup();
    
}

void loop() {
    app.loop();
}
