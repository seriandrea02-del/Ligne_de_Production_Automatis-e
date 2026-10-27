# Ligne de production miniature automatisée (ELEGOO MEGA2560 R3)

Projet personnel de conception, prototypage et validation d'un système automatisé miniature de tri et de contrôle de pièces, développé en autonomie pour mettre en pratique des compétences en systèmes embarqués et en automatisation industrielle.

---

## 💡 Objectif et Utilité du Projet

Ce projet a pour double objectif de reproduire fidèlement le comportement d'une ligne de production industrielle réelle à petite échelle et de démontrer une méthode d'ingénierie logicielle rigoureuse :
* **Maîtrise du séquencement et de la sécurité** : Gestion rigoureuse des cycles de marche/arrêt, des boucles d'arrêt d'urgence et du traitement des défauts.
* **Architecture logicielle modulaire** : Séparation stricte entre la couche matérielle et la logique algorithmique (machine à états testable unitairement sur PC).
* **Temps réel non bloquant** : Absence totale de fonctions `delay()` bloquantes dans la boucle principale pour garantir une réactivité instantanée du système[cite: 1].

---

## ⚙️ Logique et Fonctionnement

Le cœur du système repose sur une **machine à états finis (FSM) à six états**[cite: 1] :

1. **`PRET`** : État initial à la mise sous tension (aucun démarrage automatique intempestif pour des raisons de sécurité)[cite: 1]. La LED verte du convoyeur clignote lentement[cite: 1].
2. **`PRODUCTION`** : Le convoyeur est actif (LED verte fixe) et le système scrute l'arrivée des pièces[cite: 1].
3. **`TRI`** : Dès qu'une pièce est validée, sa qualité est évaluée via un potentiomètre (simulation de mesure)[cite: 1]. Le servomoteur oriente la pièce et allume l'indicateur lumineux correspondant (bleu pour conforme, jaune pour non conforme) pendant 1,5 s avant de comptabiliser la pièce[cite: 1].
4. **`ARRETE`** : Interruption propre demandée par l'opérateur (bouton STOP), figeant les actions tout en préservant l'état des compteurs[cite: 1].
5. **`DEFAUT`** : Déclenché instantanément en cas d'anomalie (ouverture de la boucle d'urgence, dépassement du temps de cycle, pièce bloquée trop longtemps, défaillance du capteur ou simulation de blocage)[cite: 1]. Le convoyeur est coupé et l'alarme visuelle/sonore s'active[cite: 1].
6. **`REINIT`** : S'enclenche lors d'un RESET volontaire[cite: 1]. *Sécurité renforcée* : le système refuse catégoriquement de quitter l'état de défaut tant que la cause matérielle n'a pas été résolue par l'opérateur[cite: 1].

### Points clés de la logique :
* **Détection anti-double comptage** : Un algorithme basé sur l'hystérésis et la confirmation sur plusieurs mesures consécutives garantit qu'une pièce maintenue devant le capteur n'est comptée qu'une seule fois[cite: 1].
* **Traçabilité et Supervision** : Chaque événement et statut est formaté et transmis via le port série (115 200 bauds) pour être journalisé sous forme de fichiers CSV par un script de supervision en Python[cite: 1].

---

## 🛠️ Architecture Matérielle & Brochage

Le prototype s'articule autour d'une carte **ELEGOO MEGA2560 R3 (ATmega2560)**[cite: 1] :

| Composant | Broche | Type / Rôle |
| :--- | :--- | :--- |
| Bouton START | `D2` | Entrée (`INPUT_PULLUP`)[cite: 1] |
| Bouton STOP | `D3` | Entrée (`INPUT_PULLUP`)[cite: 1] |
| Bouton RESET | `D4` | Entrée (`INPUT_PULLUP`)[cite: 1] |
| Bouton BLOCAGE SIMULÉ | `D5` | Entrée (`INPUT_PULLUP`)[cite: 1] |
| Boucle ARRÊT URGENCE | `D6` | Entrée (`INPUT_PULLUP`) — Sécurité positive (fil ouvert = urgence)[cite: 1] |
| HC-SR04 TRIG | `D7` | Sortie — Déclenchement de la mesure[cite: 1] |
| HC-SR04 ECHO | `D8` | Entrée — Réception de l'écho[cite: 1] |
| Servomoteur SG90 | `D9` | Sortie PWM — Aiguillage de tri[cite: 1] |
| LED Verte (Convoyeur) | `D22` | Sortie numérique[cite: 1] |
| LED Bleue (Conforme) | `D23` | Sortie numérique[cite: 1] |
| LED Jaune (Non conforme) | `D24` | Sortie numérique[cite: 1] |
| LED Rouge (Alarme) | `D25` | Sortie numérique[cite: 1] |
| Buzzer Actif | `D26` | Sortie numérique[cite: 1] |
| Potentiomètre 10 kΩ | `A0` | Entrée analogique — Qualité simulée[cite: 1] |

---

## 📁 Structure du Code Source

Le code est découplé pour isoler la logique métier du matériel :

```text
├── config.h             # Constantes, temporisations et seuils de configuration[cite: 1]
├── pins.h               # Affectation des broches de la carte[cite: 1]
├── fsm.h / fsm.cpp      # Machine à états finis (logique pure, testable sur PC)[cite: 1]
├── debouncer.h          # Anti-rebond non bloquant des boutons[cite: 1]
├── ultrasonic.h / .cpp  # Pilote non bloquant du capteur HC-SR04[cite: 1]
├── piece_detector.h     # Algorithme de détection de pièce avec hystérésis[cite: 1]
├── event_format.h / .cpp# Formatage normalisé des trames de supervision (CSV)[cite: 1]
├── ligne_production.ino # Couche matérielle Arduino et boucle principale `loop()`[cite: 1]
└── supervision/
    └── supervision.py   # Script Python de supervision, tableau de bord et export CSV[cite: 1]# Ligne_de_Production_Automatis-e
