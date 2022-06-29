

## Choix du microcontrôleur : Arduino Pro Mini 3.3V (AtMega328P)

Il a été décidé assez rapidement de rester sur des microcontrôleurs Arduino, pour respecter des contraintes de prix (microcontrôleur très générique donc peu chère) et de simplicité de fabrication (tout fablab ou bidouilleur possède un Arduino).

L'Arduino pro-mini est le format d'Arduino le plus petit, et consommant le moins d'énergie, c'est donc le plus adapté pour cette application. Un autre format de carte Arduino est tout à fait possible (il s'agit toujours du même microcontrôleur : le AtMega328P), mais au prix d'un plus grand volume et d'une consommation, donc d'une autonomie, plus faible (voir https://riton-duino.blogspot.com/2018/02/arduino-pro-mini-basse-consommation.html pour plus d'information sur la consommation).

Lors du choix de la carte électronique, il faut également faire attention au type de logique : 3.3V ou 5V. Tous les capteurs/actionneurs/périphériques devront fonctionner selon le voltage choisi (pour plus d'informations : https://rootsaid.com/arduino-logic-level/). Ici il a été choisi d'utiliser un Arduino pro mini 3.3V, donc tous les capteurs reliés se devaient de fonctionner en 3.3V.
Concernant la consommation énergétique, bien que la différence soit faible entre un Arduino pro-mini 3.3V et un 5V, il est sûrement moins couteux en batterie d'avoir une alimentation continue 3.3V que 5V. Pour plus d'informations sur la différence entre un Arduino pro-mini 3.3V et 5V, voir : https://riton-duino.blogspot.com/2020/03/arduino-pro-mini-5v-ou-33v.html.

Le choix d'un Arduino pour faire du LoRaWan n'est pas sans conséquence : pour respecter le protocole LoRaWan, beaucoup de code (venant du framework, arduino-LMIC pour nous) est nécessaire et les limites de mémoire de la puce AtMega328P sont vite atteintes. Les fonctions réalisées par un noeud Lora se doivent donc d'être simple, le code est donc assez minimaliste, ce qui rend malheureusement la prise en main plus complexe ([LMIC-node](https://github.com/lnlp/LMIC-node), est une surcouche de arduino-LMIC permettant une prise en main facilité, mais malheureusement trop couteuse en mémoire pour un Arduino).

## Choix du framework LoRaWan

Pour développer le programme d'un noeud LoRaWan, il existe principalement deux frameworks : [LoraMac-node](https://github.com/Lora-net/LoRaMac-node) et LMIC. MCCI à développé une version de LMIC fonctionnant sur carte Arduino, alors qu'il n'y a pas de version de LoraMac-node fonctionnant sur Arduino. Le choix s'est donc naturellement porté vers [arduino-LMIC](https://github.com/mcci-catena/arduino-lmic).

Une version légèrement modifiée de arduino-LMIC est utilisée, proposé par [d-a-v](https://github.com/mcci-catena/arduino-lmic/pull/756) (c'est un "hack" ne rajoutant que quelques lignes de code), permettant d'ajouter un mode économie d'énergie, nécessaire pour avoir une autonomie satisfaisante (voir [Autonomie](#autonomie)).

Néanmoins, arduino-LMIC reste un framework lourd pour une carte Arduino. Les limites de mémoire du microcontrôleur de l'Arduino, l'Atmega328P, font que les programmes utilisant le framework arduino-LMIC se doivent d'être minimaliste, rendant la prise en main du code plus complexe.

Pendant un temps, les programmes de [CongDucPham](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino) ont été utilisés (une partie du code développé est d'ailleurs basé sur son travail), car ils permettent une utilisation de LoRaWan en single-channel. Ils sont également intéressants pour comprendre les étapes du protocole LoRaWan. Néanmoins, il était souhaitable de fonctionner en multi-channel, et les programmes de CongDucPham se sont pas 100% compatibles avec la couche MAC de LoRaWan (l'OTAA par exemple, n'est pas implémenté).

Le framework arduino-LMIC permet de créer des noeuds LoRaWan de classe A (voir [spécification de LoRaWan). La classe B est implémentée, "mais non testée". La classe C, elle, n’est pas développé.
Si l’implémentation de nœud en classe C vous intéresse (pour réaliser des actionneurs par exemple), il existe le framework  [Beelan-LoRaWAN](https://github.com/BeelanMX/Beelan-LoRaWAN), que je n’ai pas testé.

##Présentation des programmes présents. Comment les utiliser, les modifier

Tous ces programmes utilisent l’OTAA, et respecte le protocole LoRaWan pour les appareils de classe A (c’est du "LoRaWan pur", la couche MAC est implémentée). Ils sont basés sur le programme [ttn-otaa-feather-us915](https://github.com/mcci-catena/arduino-lmic/tree/master/examples) de terrillmoore, pour lmic-arduino, et le programme [Arduino_LoRa_Generic_DHT](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino) de CongducPham, pour LowCostLoRaGw. En plus de cette fusion, d’autres ajouts ont été faits, comme la gestion du mode d’économie d’énergie, et la gestion des interruptions.

### OTAA_DHT11
Programme permettant le relevé de la température et de l’humidité à l’aide d’un DHT11. Ce programme peut servir de base pour développer d’autres programmes pour d’autres capteurs.

### OTAA_DHT11_Voltage

En plus d'un DHT11, un relevé d'une tension est réalisé sur un pin analogique. Ce programme nécessite l’utilisation d’un diviseur de tension, de telle sorte que la tension maximum mesurée soit égale à 3.3V (soit la tension d’alimentation de l’Arduino Pro-mini). Les valeurs des résistances utilisées doivent être éditées dans le fichier VOLT.cpp :
```c++
#define R1 51000
#define R2 100000
```
Voir <https://www.instructables.com/Voltage-Measurement-Using-Arduino/> pour plus d’explication sur la méthode de mesures.
Pour comprendre comment une mesure « propre » du voltage est réalisée, j’invite également à lire ceci : <https://hackingmajenkoblog.wordpress.com/2016/02/01/making-accurate-adc-readings-on-the-arduino/>.
Ce programme a été utilisé pour faire des tests en conditions réelles de durée de vie de la batterie de la carte développée.

### HCSR04
Programme permettant de mesurer une distance, à l’aide d’un capteur ultrason HCSR04-P. On précise bien qu’il s’agit d’un HCSR04-P, et pas d’un HCSR04, car ce dernier ne peut pas fonctionner correctement en 3.3V.	

## Modification des capteurs relevés

Pour modifier la liste des capteurs réalisant une mesure pour le noeud, il faut modifier la liste des capteurs :

```
// Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger=-1)
sensor_ptrs[0] = new DHT11_Temperature('T', IS_NOT_ANALOG, IS_CONNECTED, low_power_status, 5,4);
sensor_ptrs[1] = new DHT11_Humidity('H', IS_NOT_ANALOG, IS_CONNECTED, low_power_status, 5,4);
sensor_ptrs[2] = new VOLT('V', IS_NOT_ANALOG, IS_CONNECTED, low_power_status, A0);
```
Ainsi que le nombre de capteur(s) connecté(s) :

```c++
const int number_of_sensors = 3;
```


## Ajout d’un nouveau capteur

Si vous n’avez jamais eu à faire à de la programmation objet, ça risque d’être un peu compliqué à prendre en main.
L’implémentation de nouveau capteurs est simplifié par la définition d’une classe Sensor, classe mère de toute classe de capteur. Ainsi dans la plupart des cas, il suffit de définir la nouvelle classe de capteur comme classe fille de Sensor (dans le fichier .h, exemple pour VOLT.h) :

```c++
#ifndef VOLT_H
#define VOLT_H

#include "Sensor.h"

class VOLT : public Sensor {
  public:
    VOLT(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read);
    double get_value();
    void update_data();
    long readVcc();
};

#endif
```

Puis, dans le fichier .cpp du nouveau capteur, définir le constructeur et les deux méthodes update_data et get_value (exemple de VOLT.cpp) :

```c++

VOLT::VOLT(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read){
  if (get_is_connected()){
    
  set_warmup_time(0);
  }
}

void VOLT::update_data()
{
  if (get_is_connected()) {

    int readPin = get_pin_read();

    long vcc = readVcc();
    double voltage = analogRead(readPin)* (vcc*0.001/1024.0) * ((R1 + R2)/R2);

    set_data(voltage);

  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((float)random(0, 450));
  }

}

double VOLT::get_value()
{
  uint8_t retry=4;
  
  do { 

    update_data();
    Serial.println(get_data());

    retry--;
    
  } while (retry && get_data()==-1.0);

  return get_data();
}

```

## Pin mapping [schéma]


## Système d’interruption

Quel est l'intérêt des interruptions ? Une interruption permet de lancer une "routine" (entendre fonction) prioritaire, en cas de déclenchement (voir https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/ pour une explication plus complète sur le principe).
L'utilisation des interruptions parait pertinente pour les noeuds Lora : dans le cas où un noeud a pour fonction d'attendre un stimuli d'un capteur (et pas de faire un relevé "analogique"), s'il n'y avait pas les interruptions, il faudrait vérifier en permanence le capteur, ce qui serait très énergivore (pas souhaitable pour un capteur autonome sur batterie), ou alors programmer des relevés à certains moments uniquement, au prix d'un manque de réactivité et au risque de manquer l'information. Les interruptions permettent le parfait compromis : l'Arduino peut être en mode LowPower, tout en étant capable d'envoyer un uplink dès qu'il y aura un stimuli sur un pin.

Les programmes développés permettent l’utilisation des deux pins d’interruptions de l’Arduino Pro-mini (les pins 2 et 3). Leur utilisation, c’est-à-dire l’envoi d’un paquet lorsqu’il y a une réaction sur un de ses pins, se fait en définissant :
```c++
#define IRQ_PIN2
#define IRQ_PIN3
```

## Utilisation de PlatformIO ou de l’IDE Arduino

À un certain stade du développement, il a été décidé d’utiliser [PlatformIO](https://docs.platformio.org/en/latest/) plutôt que l’IDE Arduino, pour rendre le développement plus agréable. L’IDE Arduino permet une première approche de la programmation, mais cet IDE est très simple et manque de nombreuses fonctions que l’on retrouve dans des IDE plus "professionnels". L’utilisation de PlatformIO permet de gérer la partie compilation, téléversement et communication avec une carte Arduino (ou avec d’autres microcontrôleurs, un avantage pour le cross-platform). Cela se présente sous la forme d’un plugin (ou d’une CLI) présent pour des IDE populaires et reconnus.

Il est tout à fait possible de travailler sur l’IDE Arduino. Pour cela, il faut :
* Ajouter la librarie LMIC modifiée *LMIC_LowPower* , dans le dossier des libraries de l’Arduino
* Installer la librarie LowPower
* Mettre le contenu du dossier *src* dans la racine du projet (un niveau au-dessus)
* Renommer le fichier principal *main.cpp* par le nom de dossier du projet (si les programmes se trouvent dans le dossier *NoeudLorawan_lmic_lowpower*, *main.cpp* doit être renommé *NoeudLorawan_lmic_lowpower.ino*)


## Autonomie

De nombreuses méthodes existent pour augmenter l'autonomie d'un Arduino (voir [ceci](https://riton-duino.blogspot.com/2018/02/arduino-pro-mini-basse-consommation.html) ou [cela](http://www.gammon.com.au/power)). La plus significative est l'utilisation d'un mode d'économie d'énergie, quand l'Arduino est en attente. Il est fondamental d'implémenter cela pour l'utilisation en IOT, car la majorité du temps, les appareils sont inactifs (un relevé se fera généralement en moins de 10s, et tout les 5min maximum, ce qui fait qu'un appareil n'est finalement actif que 3% du temps). 

Dans ce travail, nous nous sommes concentré sur l’implémentation d'un mode d'économie d'énergie avec le framework arduino-LMIC (c'est une sorte de "hack" du framework). La librarie LowPower est alors utilisée pour gérer la partie économie d'énergie. D'autres méthodes, que ce soit matériel ou logiciel, pourraient être ajoutées pour améliorer l'autonomie. La consommation en mode économie d'énergie, lors de tests, était de l'ordre de 2mA, ce qui semblait suffisamment satisfaisant pour un appareil "lowcost" : pour 2 piles AA Ni-Mh, avec un élévateur de tension externe 3.3V, la batterie pourrait durer au moins 2/3 mois, théoriquement (voir ce [site](http://oregonembedded.com/batterycalc.htm) pour estimer la durée de vie d'une batterie et comprendre le calcul).
Peu de tests ont pu être réalisés pour s'assurer de l'autonomie réelle. Un test a montré qu'une noeud monté avec un DHT11, 2 piles Ni-Mh de 2800mAh chacunes, et avec un envoie de paquet toutes les 20s, pouvait tenir une dizaine de jours. Avec une utilisation normale (c'est-à-dire avec un envoie de paquet beaucoup moins régulier, donc le module serait en mode économie d'énergie la majorité du temps), on pourrait espérer avoir une autonomie 2 voir 3 fois plus élevée, si ce n'est mieux. Je n'ai pas de doute qu'on puisse obtenir une autonomie de 1 mois avec ce module, et une autonomie de plusieurs mois avec quelques améliorations (meilleurs piles ? Plus d'optimisation en mode économie d'énergie ? Choix d'autres composants électroniques ? Suppression de la LED de l’Arduino pro-mini ?). 

## Codecs

Pour transmettre des informations entre les noeuds Arduino et Chirpstack, il est important de s'accorder sur la nomenclature des paquets (on parle uniquement de la partie "donnée" des paquets, le "payload", pas des en-têtes ajoutés par les couches réseaux). Il faut donc définir les algorithmes de codage et décodage des informations. Le but étant d'obtenir, au bout de la chaine, un JSON pour chaque paquet, avec une ligne pour chaque variable (au format "clé : valeur", exemple : {"temperature": 22.5} ).

À noter que les paquets sont également encodés puis décodés en base64, entre la transmission par la gateway sur MQTT et la réception par Chirpstack-Network-Server (important d'avoir cette info lorsque l'on débogue les paquets dans cette couche réseau !).

CayenneLPP est une librairie permettant un formatage simple des données pour l'encodage/décodage en LoRaWan : <https://www.thethingsindustries.com/docs/integrations/payload-formatters/cayenne/>
Le travail de CongducPham, sur les programmes des cartes Arduino, proposait aussi un format de données simple, en utilisant des strings (les valeurs numériques int ou float sont convertis en strings puis placé dans le paquet) :
```
\!TC/27.79/HU/56.50
```
Cependant, l'utilisation du framework LMIC et d'un Arduino (Atmega328P) font que la quantité de mémoire disponible pour ajouter du code (en plus de celui fournit par LMIC, permettant de respecter le protocole LoRaWan) est très faible. Le code se devait d'être le plus minimaliste possible pour pouvoir être téléversé en entier sur la carte Arduino. L'utilisation de la librairie CayenneLPP ou de la librairie String prenait beaucoup de place, il a donc été décidé de créer des payloads ultra minimaliste pour pouvoir conserver plus de mémoire. Le format des payloads été donc le suivant :
```
0T27.79H56.50

IT27.79H56.50
```
* Le 1er caractère (0 ou I) sert à détecter s'il y a eu une interruption ou non
* Il n'y a pas de séparateur entre les différents éléments, car les tailles sont fixes : un unique caractère est utilisé pour définir la variable, soit un octet (T pour température, H pour humidité), et la valeur suivante est toujours un float, soit 4 octets. Le paquet *0T27.79H56.50* est donc composé de 1 + 1 + 4 + 1 + 4 = 11 octets, et ce sera toujours le cas quelque-soit la mesure.

Pour décoder les paquets sur Chirpstack en JSON, un programme en javascript est nécessaire (ceux développés sont dans le dossier [codecJS](./codecJS) . Le parsing de paquets bas niveau (c'est-à-dire composé de type de variables simples, char et float, au lieu d'une string) n'est vraiment pas simple en javascript, le programme peut donc sembler lunaire, mais il fonctionne, et permet d'utiliser des paquets ultra minimaliste !
