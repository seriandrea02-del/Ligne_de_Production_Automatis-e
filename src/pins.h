// pins.h — Affectation des broches ELEGOO MEGA2560 R3 (voir tableau dans README et rapport).
// ATTENTION: aucune broche ne doit apparaître deux fois (vérifié par tests/check_pins.py).
#pragma once
#include <Arduino.h>
 
// Entrées numériques (boutons câblés vers GND, INPUT_PULLUP: appuyé = LOW)
#define PIN_BTN_START      2
#define PIN_BTN_STOP       3
#define PIN_BTN_RESET      4
#define PIN_BTN_FAULT_SIM  5
#define PIN_ESTOP_LOOP     6   // boucle "NC" vers GND: fermée = normal, OUVERTE = urgence
 
// Capteur ultrason HC-SR04
#define PIN_US_TRIG        7
#define PIN_US_ECHO        8
 
// Servomoteur de tri (SG90)
#define PIN_SERVO          9
 
// Sorties numériques (LED + résistance 220 ohms en série; buzzer actif)
#define PIN_LED_CONV      22   // vert: convoyeur (moteur simulé)
#define PIN_LED_OK        23   // bleu: pièce conforme
#define PIN_LED_NOK       24   // jaune: pièce non conforme
#define PIN_LED_ALARM     25   // rouge: alarme
#define PIN_BUZZER        26
 
// Entrée analogique: potentiomètre = mesure qualité simulée
#define PIN_QUALITY_POT   A0
 
