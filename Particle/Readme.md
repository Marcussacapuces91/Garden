## Code source C++ pour Particle

### Présentation

Il s'agit du code qui doit être compilé et transmis vers le Particle Photon.

Sur le Photon, sont branchés :
- un capteur de luminosité BH1750 http://rohmfs.rohm.com/en/products/databook/datasheet/ic/sensor/light/bh1750fvi-e.pdf
- un capteur de température et d'humidité DHT22 https://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf
- deux Mosfet pour commuter des charges importantes comme une barette de leds ou une pompe.
 
### Class App
C'est la classe qui implémente toutes les fonctions de l'application et en particulier 
- setup()
- loop()
qui sont les deux fonctions à utiliser dans le framework de Particle. 


