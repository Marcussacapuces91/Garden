/**
 * Copyright (c) 2015 par Marc Sibert.
 * 
 * Ce fichier fait partie de Garden V2.
 *
 * Garden V2 est un logiciel libre ; vous pouvez le redistribuer ou le modifier 
 * suivant les termes de la GNU General Public License telle que publiée par la 
 * Free Software Foundation ; soit la version 3 de la licence, soit (à votre 
 * gré) toute version ultérieure.
 * Garden V2 est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE 
 * GARANTIE ; sans même la garantie tacite de QUALITÉ MARCHANDE ou d'ADÉQUATION 
 * à UN BUT PARTICULIER. Consultez la GNU General Public License pour plus de 
 * détails.
 * Vous devez avoir reçu une copie de la GNU General Public License en même 
 * temps que Garden V2 ; si ce n'est pas le cas, 
 * consultez <http://www.gnu.org/licenses>.
 **/
 
// SYSTEM_THREAD(ENABLED);

#include "app.h"

App app;

void setup() {
    app.setup();
    
}

void loop() {
    app.loop();
}
