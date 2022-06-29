

## Choix du microcontrôleur

Il a été décidé assez rapidement de rester sur des microcontroleurs Arduino, pour respecter des contraintes de prix (microcontroleur très générique donc peu chère) et de simplicité de fabrication (tout fablab ou bidouilleur possède un arduino).

L'Arduino pro-mini est le format d'Arduino le plus petit, et consommant le moins d'énergie, c'est donc le plus adapté pour cette application. Un autre format de carte Arduino est tout à fait possible (il s'agit toujours du même microcontroleur : le AtMega328P), mais au prix d'un plus grand volume et d'une consommation, donc d'une autonomie, plus faible (voir https://riton-duino.blogspot.com/2018/02/arduino-pro-mini-basse-consommation.html pour plus d'information sur la consommation).

Lors du choix de la carte électronique, il faut également faire attention au type de logique : 3.3V ou 5V. Tout les capteurs/actionneurs/périphériques devront fonctionner selon le voltage choisi (pour plus d'informations : https://rootsaid.com/arduino-logic-level/). Ici il a été choisi d'utiliser des arduino pro mini 3.3V, donc tout les capteurs reliés se devait de fonctionner en 3.3V.
Concernant la consommation énergétique, bien que la différence soit faible entre un Arduino pro-mini 3.3V et un 5V, il est sûrement moins couteux en batterie d'avoir une alimentation continue 3.3V que 5V. Pour plus d'informations sur la différence entre un Arduino pro-mini 3.3V et 5V, voir : https://riton-duino.blogspot.com/2020/03/arduino-pro-mini-5v-ou-33v.html.

Le choix d'un Arduino pour faire du LoRaWan n'est pas sans conséquence : pour respecter le protocole LoRaWan, beacoup de code (venant du framework, arduino-LMIC pour nous) est nécessaire et les limites de mémoire de la puce AtMega328P sont vites atteintes. Les fonctions réalisés par un noeud Lora se doivent donc d'être simple, le code est donc assez minimaliste, ce qui rend malheureusement la prise en main plus complexe (LMIC-node, est une surcouche de arduino-LMIC permettant une prise en main facilité, mais malheureusement trop couteuse en mémoire pour un Arduino).

## Choix du framework LoRaWan

Pour développer le programme d'un noeud LoRaWan, il existe principalement deux frameworks : LoraMac-node et LMIC. MCCI à développé une version de LMIC fonctionnant sur carte Arduino, alors qu'il n'y a pas de version de LoraMac-node fonctionnant sur Arduino. Le choix s'est donc naturellement porté vers LMIC, ou plutôt arduino-LMIC.

Une version légèrement modifié de arduino-LMIC est utilisée, proposé par d-a-v (https://github.com/mcci-catena/arduino-lmic/pull/756, c'est un "hack" ne rajoutant que quelques lignes de code), permettant d'ajouter un mode économie d'énergie, nécessaire pour avoir une autonomie satisfaisante (voir [partie sur l'autonomie]).

Néanmoins, arduino-LMIC reste un framework lourd pour une carte Arduino. Les limites de mémoire du microcontrolleur de l'Arduino, l'Atmega328P, font que les programmes utilisant le framework arduino-LMIC se doivent d'être minimaliste, rendant la prise en main du code plus complexe.

Pendant un temps, les programmes de CongDucPham ont été utilisés (une partie du code développé est d'ailleurs basé sur son travail), car ils permettent une utilisation de LoRaWan en single-channel. Ils sont également intéressant pour comprendre les étapes du protocole LoRaWan. Néanmoins, il était souhaitable de fonctionner en multi-channel, et les programmes de CongDucPham se sont pas 100% compatibles avec la couche MAC de LoRaWan (l'OTAA par exemple, n'est pas implémenté).

Le framework arduino-LMIC permet de créer des nœud LoRaWan de classe A (voir [spécification de LoRaWan). La classe B est implémentée, « mais non testée ». La classe C, elle, n’est pas développé.
Si l’implémentation de nœud en classe C vous intéresse (pour réaliser des actionneurs par exemple), il existe le framework Beelan-LoRaWAN (https://github.com/BeelanMX/Beelan-LoRaWAN), que je n’ai pas testé.

##Présentation des programmes présents. Comment les utiliser, les modifier

Tout ces programmes utilisent l’OTAA, et respecte le protocole LoRaWan pour les appareils de classe A (c’est du « pure LoRaWan », la couche MAC est implémentée). Ils sont basé sur le programme ttn-otaa-feather-us915 de terrillmoore, pour lmic-arduino, et le programme Arduino_LoRa_Generic_DHT de CongducPham, pour LowCostLoRaGw. [mettre des références plus propres]. En plus de cette fusion, d’autres ajouts ont été fait, comme la gestion du mode d’économie d’énergie, et la gestion des interruptions.

###DHT11
Programme permettant le relevé de la température et de l’humidité à l’aide d’un DHT11. Ce programme peut servir de base pour développer d’autres programmes pour d’autres capteurs.

### Voltage
Programme permettant le relevé d’une tension. Ce programme nécessite l’utilisation d’un diviseur de tension, de telle sorte que la tension maximum mesurée soit égale à 3.3V (soit la tension d’alimentation de l’Arduino Pro-mini).  Les valeurs des résistances utilisées doivent être éditée dans le programme src/VOLT.cpp):[screen du code]

Voir https://www.instructables.com/Voltage-Measurement-Using-Arduino/ pour plus d’explication sur la méthode de mesures.
Pour comprendre comment une mesure « propre » du voltage est réalisée, j’invite également à lire [https://hackingmajenkoblog.wordpress.com/2016/02/01/making-accurate-adc-readings-on-the-arduino/].

###DHT11_Voltage
Combinaison des deux programmes précédents, permettant à la fois le relevé la température et l’humidité avec un DHT11, et de surveiller une tension. Ce programme a été utiliser pour faire des tests en condition réelles de durée de vie de la batterie de la carte développée.

#HCSR04
Programme permettant de mesurer une distance, à l’aide d’un capteur ultrasion HCSR04-P. On précise bien qu’il s’agit d’un HCSR04-P, et pas d’un HCSR04, car ce dernier ne peut pas fonctionner correctement en 3.3V.	

##Modification des capteurs relevés

Pour modifier la liste des capteurs qui réalisent une mesure pour un nœud, il faut modifier les [lignes suivantes] dans main.cpp. [ligne nombre de capteur, liste de def des fichiers capteurs, ET de def du tableau sensors_ptrs].

## Ajout d’un nouveau capteur

Si vous n’avez jamais eu à faire à de la programmation objet, ça risque d’être un peu compliqué à prendre en main.
L’implémentation de nouveau capteurs est simplifié par la définition d’une classe Sensor, classe mère de toute classe de capteur. Ainsi dans la plupart des cas, il suffit de définir la nouvelle classe de capteur comme classe fille de Sensor (dans le fichier .h) : [screen code DHT11.h exemple]. Puis, dans le fichier .cpp du nouveau capteur, définir le constructeur et les deux méthodes update_data et get_value [screen exemple DHT11].

## Pin mapping [schéma]


## Système d’interruption

Quel est l'intérêt des interruptions ? Une interruption permet de lancer une "routine" (entendre fonction) prioritaire, en cas de déclenchement (voir https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/ pour une explication plus complète sur le principe).
L'utilisation des interruption parait pertinent pour les noeuds Lora : dans le cas où un noeud a pour fonction d'attendre un stimuli d'un capteur (et pas de faire un relevé 'analogique'), s'il n'y avait pas les interruptions, il faudrait vérifier en permanence le capteur, ce qui serait très énergivore (pas souhaitable pour un capteur autonome sur batterie), ou alors programmer des relevés à certains moment uniquement, au prix d'un manque de réactivité et au risque de manquer l'information. Les interruptions permettent le parfait compromis : l'arduino peut être en mode LowPower, tout en étant capable d'envoyer un uplink dès qu'il y aura un stimuli sur un pin.

Les programmes développées permettent l’utilisation des deux pins d’interruptions de l’Arduino Pro-mini (les pins 2 et 3). Leur utilisation, c’est à dire l’envoi d’un packet lorsqu’il y a une réaction sur un de ses pins, se fait en définissant [#define IRQ_PIN2 ou #define IRQ_PIN3]

## Utilisation de platformIO ou de l’IDE Arduino

À un certain stade du développement, il a été décidé d’utiliser platformIO plutôt que l’IDE Arduino, pour rendre le dévellopement plus agréable. L’IDE Arduino permet une première approche de la programmation, mais cet IDE est très simple et manque de nombreuses fonctions que l’on retrou
ve dans des IDE plus « professionnel ». L’utilisation de platformIO permet de gérer la partie compilation, téléversement et communication avec une carte Arduino (ou avec d’autres microcontrolleur, un avantage pour le cross-platform). Cela se présente sous la forme d’un plugin (ou d’une CLI) présent pour des IDE populaires et reconnus.

Il est tout à fait possible de travailler sur l’IDE Arduino. Pour cela, il faut :
- ajouter la librarie LMIC modifié « LMIC_LowPower », dans le dossier des libraries de l’Arduino
- ajouter la librarie LowPower
- mettre le contenu du dossier « src » dans la racine du projet (un niveau au dessus)
- renommer le fichier principale « main.cpp » par le nom de dossier du projet (si les programmes se trouve dans le dossier « NoeudLorawan_lmic_lowpower », main.cpp doit être renommé NoeudLorawan_lmic_lowpower.ino)


## Méthodes pour améliorer l'autonomie

De nombreuses méthodes existent pour augmenter l'autonomie d'un arduino :
voir https://riton-duino.blogspot.com/2018/02/arduino-pro-mini-basse-consommation.html ou encore http://www.gammon.com.au/power). La plus significative est l'utilisation d'un mode d'économie d'énergie, quand l'Arduino est en attente. Il est fondamentale d'implémenter cela pour l'utilisation en IOT, car la majorité du temps, les appareils sont inactifs (un relevé se fera généralement en moins de 10s, et tout les 5 min maximum, ce qui fait qu'un appareil est finalement actif que 3% du temps (10/(5*60))). 

Dans ce travail, nous nous sommes concentré sur l’implémentation d'un mode d'économie d'énergie avec le framework arduino-LMIC (c'est une sorte de "hack" du framework). La librarie LowPower est alors utiliser pour gérer la partie économie d'énergie. D'autres méthodes, que ce soit matérielle ou logiciel, pourraient être ajoutée pour améliorer l'autonomie. La consommation en mode économie d'énergie, lors de tests, était de l'ordre de 2mA, ce qui semblait sufisamment satisfaisant pour un appareil lowcost : pour 2 piles AA Ni-Mh, avec un élévateur de tension externe 3.3V, la batterie pourrait durer au moins 2/3 mois, théoriquement (voir ce site pour estimer la durée de vie d'une batterie et comprendre le calcul : http://oregonembedded.com/batterycalc.htm).
Peu de tests ont pu être réalisé pour s'assurer de l'autonomie réelle. Un test a montré qu'une carte [lora-her], monté avec un DHT11, 2 piles Ni-Mh de 2800mAh chacunes, et avec un envoie de paquet toutes les 20s, pouvait tenir une dizaine de jours. Avec une utilisation normale (c'est à dire envoie de paquet beacoup moins régulier, donc le module serait en mode économie d'énergie la majorité du temps), on pourrait espérer avoir une autonomie 2 voir 3 fois plus élevée, si ce n'est mieux. Je n'ai pas de doute qu'on puisse obtenir une autonomie de 1 mois avec ce module, et une autonomie de plusieurs mois avec quelques améliorations (meilleurs piles ? Plus d'optimisation en mode économie d'énergie ? Choix d'autres composants électroniques ? Suppression de la LED de l’Arduino pro-mini ?). 

## Codecs

Pour transmettre des informations entre les noeuds Arduino et Chirpstack, il est important de s'accorder sur la nomenclature des paquets (on parle uniquement de la partie données des paquets, le "payload", pas des en-têtes ajoutés par les couches réseaux). Il faut donc définir les algorithmes de codage et décodage des informations. Le but étant d'obtenir, au bout de la chaine, un JSON pour chaque paquet, avec une ligne pour chaque variable (au format "clé : valeur", exemple : {"temperature": 22.5} ).

À noter que les paquets sont également encodés puis décodés en base64, entre la transmission par la gateway sur MQTT et la réception par Chirpstack-Network-Server (important d'avoir cette info lorsque l'on débogue les paquets dans cette couche réseau !).

CayenneLPP est une librairie permettant un formatage simple des données pour l'encodage/décodage en LoRaWan : https://www.thethingsindustries.com/docs/integrations/payload-formatters/cayenne/
Le travail de CongducPham, sur les programmes des cartes Arduino, proposait aussi un format de données simple, en utilisant des strings (les valeurs numériques int ou float sont convertis en strings puis placé dans le paquets) -> \!TC/27.79/HU/56.50 
Cependant, l'utilisation du framework LMIC et d'un Arduino (Atmega328P) font que la quantité de mémoire disponible pour ajouter du code (en plus de celui fournit par LMIC, permettant de respecter le protocole LoRaWan) est très faible. Le code se devait d'être le plus minimaliste possible pour pouvoir être téléversé en entier sur la carte Arduino. L'utilisation de la librairie CayenneLPP ou de la librairie String prenait beacoup de place, il a donc été décidé de créer des payloads ultra minimaliste pour pouvoir conserver plus de mémoire.
Le format des payloads été donc le suivant :
0T27.79H56.50 ou IT27.79H56.50
- le 1er caractère sert à détecter s'il y a eu une interrupion ou non
- il n'y a pas de séparateur entre les différents éléments, car les tailles sont fixes : un unique caratère est utilisé pour définir la variable, soit un octet (T pour températeur, H pour humidité), et la valeur suivante est toujours un float, soit 4 octets. Le paquets "0T27.79H56.50" est donc composé de 1 + 1 + 4 + 1 + 4 = 11 octets, et ce sera toujours le cas quelque soit la mesure.

Pour décoder les paquets sur Chirpstack en JSON, un programme en javascript est nécessaire, il est fournit [à tel endroit]. Le parsing de paquets bas niveau (c'est-à-dire composé de type de variables simples, char et float, au lieu d'une string) n'est vraiment pas simple en javascript, le programme peut donc sembler lunaire, mais il fonctionne, et permet d'utiliser des paquets ultra minimaliste !