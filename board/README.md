# Carte/PCB *LoRaXHER* à assembler

Il n’existe pas beaucoup de solutions disponibles au grand public, pour créer des nœuds LoRaWan avec un microcontrôleur :

* Il existe des shields Lora pour Arduino (comme le [Dragino LoRa Arduino Shield](https://www.dragino.com/products/lora/item/102-lora-shield.html)). Ils sont intéressants pour une première expérience, mais ils sont assez chers en définitive (et ils se font de plus en plus rares).

* Il y a les cartes *Adafruit Feather*, ou celles de *MCCI Catena*, équipées d'un microcontrôleur et d'un module Lora. Les microcontrôleurs proposés ont une plus grande mémoire que l'AtMega328P et permettent d'avoir des programmes plus riches (AtMega328P de l’Arduino est assez limitant pour le LoRaWan). Les premiers prix sont autour de 30/40e, mais il faut ajouter à cela une batterie pour alimenter la carte.

* Les solutions tout-en-un les plus connus sont celles de *LiLygo*, et notamment le [LILYGO® TTGO T-Beam V1.1](http://www.lilygo.cn/prod_view.aspx?TypeId=50044&Id=1317&FId=t3:50044:3), équipé d'un ESP32, d'un module LoRa, GPS, Wifi, d'un slot pour une pile Li-ion 18650 et d'un écran Oled. Mais l'appareil n'a pas une bonne autonomie, il sert plutôt à une première approche ou pour des utilisations courtes en temps.

Il semblait donc intéressant de proposer un modèle de PCB, basé sur de l’Arduino, à assembler soi-même. Sans compter l’investissement pour la fabrication de la PCB, et le prix des capteurs à ajouter, le prix des composants nécessaires pour avoir un nœud LoRa fonctionnant avec un Arduino est de l’ordre de 20€ :

* [Antenne LoRa 868Mhz](https://fr.aliexpress.com/item/1005003202367520.html?pdp_npi=2%40dis%21EUR%21%E2%82%AC%2010%2C20%21%E2%82%AC%209%2C18%21%21%21%21%21%402100bdd716555498392237979e3114%2112000024637823214%21sh) : ~2€

* [Arduino pro-mini 3.3V](https://fr.aliexpress.com/item/32821902128.html?gatewayAdapt=glo2fra&mp=1) : ~5,2€

* [Molule Lora RFM95W 868](https://fr.aliexpress.com/item/32820746771.html?gatewayAdapt=glo2fra&mp=1) : ~5,3€

* [Connecteur SMA pour l’antenne](https://fr.aliexpress.com/item/1005003111950768.html?algo_pvid=65342ded-14d5-4c3d-974b-1c306e3e3275&algo_exp_id=65342ded-14d5-4c3d-974b-1c306e3e3275-7&pdp_ext_f=%7B%22sku_id%22%3A%2212000024162554429%22%7D&pdp_npi=2%40dis%21EUR%21%213.36%21%21%21%21%21%402100bdec16555506040421455e9535%2112000024162554429%21sea) : ~0,5€

* [Slot pour 2 piles AA](https://fr.aliexpress.com/item/32993381574.html?gatewayAdapt=glo2fra&mp=1) : ~1,3€

* [Élévateur de tension 3.3V](https://fr.aliexpress.com/item/32800430445.html?gatewayAdapt=glo2fra&mp=1) : ~2,3€

* 2 piles AA Ni-Mh de qualités : ~5€

* Quelques résistances, 2 LED et un condensateur : ~0€ !

**Total : ~ 22€**

## Modules Lora utilisés

* SX1276 : module Lora produit par Semtech, fonctionnant sur les bandes de fréquences 868MHz et 915MHz. C'est le module a utiliser en Europe (pour être dans la bande 868MHz). 

* RFM95/RFM95W : copie du SX1276 de Semtech. Un peu moins chère donc à privilégier !

* SX1278 : module Lora produit par Semtech, fonctionnant sur la bande 433MHz. Il s'agit de la bande de fréquences autorisée en Asie, mais c'est également une bande de fréquences utilisée par les radioamateurs en Europe (Région 1). Donc il est possible d'utiliser ces modules Lora en étant radioamateur.


**Attention** : Le pin-mapping entre les modules 868MHz et 433MHz est différent ! La carte *LoRaXHER* n'est donc compatible qu'avec les modules 868MHz.

## PCB *LoRaXHER*
Le modèle de la PCB se veut simple à réaliser soi-même ; c’est un circuit 1 couche, avec des tracks épaisses et pas de composant de surface à souder (sauf le module LoRa). Les PCBs réalisées à l’Hermitage ont été faites avec une CNC entrée de gamme, tout fablab devrait être en mesure de les fabriquer !

La PCB réalisé permet donc d’accéder aux pins digitaux et analogiques d’un Arduino pro-mini, pour brancher d’autres composants :

* Les pins RX/TX sont accessibles et permettent à la fois de téléverser du code avec une carte FTDI, soit de connecter un autre composant (1)
* Le pin digital 2 est relié a 2 jumpers mâles, qui permettent de détecter des interruptions (2). Une LED peut également être connecté grâce à un jumper ; elle s’allumera lorsque le courant passe entre les 2 jumpers d’interruptions (elle est connectée de manière analogique, pas par l’intermédiaire du programme de l’Arduino) (3)
* Une autre LED est reliée au pin 5, elle s’allumera si le pin est HIGH. (4)

**[photo board avec zones correspondants à explications au-dessus]

[photo board + cnc]**

## Alimentation de la carte

Pour alimenter un Arduino pro-mini il y a deux solutions :                              
* Fournir une tension supérieure à sa tension d'alimentation en passant par le pin RAW. La tension passe alors par un régulateur interne pour atteindre cette tension d'alimentation (3.3V ou 5V).                                              
* Fournir directement la bonne tension d'alimentation (3.3V ou 5V), en passant par le pin VCC. Le régulateur interne de l'Arduino n'est alors pas utilisé, et il faut utiliser une solution externe à la carte pour obtenir la bonne tension. 
                                                                               
Il est assez commun d'acheter des batteries 9V pour de premiers projets Arduino. C'est en fait une mauvaise idée si on veut une bonne autonomie, la solution idéale étant plutôt d'utiliser des piles AA rechargeables, et de les combiner avec un élévateur de tension externe 3.3V (ou 5V), plus efficace que le régulateur de tension 3.3V intégré dans l'Arduino (voir [ceci](https://cybergibbons.com/arduino/arduino-misconceptions-6-a-9v-battery-is-a-good-power-source/) pour plus d'explications).
Pendant un temps, pour éviter d'avoir à acheter un élévateur de tension 3.3V en plus d'un Arduino, il a également été envisager d'utiliser des piles Li-Ion (3.7V) avec le régulateur interne de l'Arduino (pour passer en 3.3V), ou d'utiliser directement sans régulateur des piles LiFePo4 en 3.2V. Mais les piles AA Ni-Mh sont bien plus intéressantes en matière de prix, de capacité et plus simples à trouver.

La PCB développé permet donc de brancher un élévateur de tension, puis d’alimenter directement le pin VCC de l’Arduino (a). Cependant un jumper est présent pour choisir d’alimenter plutôt le pin RAW est de passer par le régulateur interne (b).

**[photo board avec flèche sur le jumper]**

## Portée du signal LoRa

Plusieurs paramètres dans la conception de la PCB ont été pensé pour avoir une bonne transmission du signal radio dans la carte :

* Utilisation de connecteurs sma à souder directement sur la carte, plutôt que des connecteurs ipex (plus facile à souder et plus facile de conserver un bon signal)
* Utilisation d’un plan de masse, pour limiter les perturbations
* Ligne de transmission courte, entre la sortie du module Lora et l'antenne, pour éviter d'avoir à faire de l'adaptation d'impédance (voir [ceci](https://electronics.stackexchange.com/questions/207532/does-my-pcb-require-a-50-ohm-impedance-trace-even-if-i-am-using-an-external-ante) et [cela](https://en.wikipedia.org/wiki/Impedance_matching#Transmission_lines))

Peu de tests ont pu être réalisés, il est difficile de dire si ces paramètres ont une influence (positive ou négative) sur la qualité du signal et sur la portée entre un nœud et sa passerelle. Des avis d’expert dans ce domaine seraient le bienvenu !
Il semble également qu’il soit plus difficile pour un nœud de recevoir un downlink que pour la passerelle de recevoir un uplink. En effet il arrive que l’OTAA rate car le nœud ne reçoit pas le paquet de confirmation de la passerelle. Il serait intéressant de faire des tests de porté concentrés sur les downlinks ; faut-il une autre PCB pour pouvoir assurer la réception des downlinks ? C’est une des raisons pour laquelle le développement des nœuds de Classe C (les actionneurs) a été mise de côté.
