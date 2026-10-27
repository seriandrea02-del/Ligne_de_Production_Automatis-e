// config.h — Constantes de configuration (indépendantes du matériel, testables sur PC)
// Projet : Ligne de production miniature — Version A (ELEGOO MEGA2560 R3)
#pragma once
#include <stdint.h>
 
namespace Cfg {
// --- Anti-rebond des boutons ---
const uint16_t DEBOUNCE_MS = 30;
 
// --- Capteur ultrason HC-SR04 (détection de présence d'une pièce) ---
const uint32_t US_PERIOD_US       = 40000UL;  // 40 ms entre deux mesures (> 60 ms recommandé par certains datasheets: voir README)
const uint32_t US_RISE_TIMEOUT_US = 3000UL;   // si ECHO ne monte pas dans ce délai -> capteur absent/débranché
const uint32_t US_MAX_ECHO_US     = 30000UL;  // écho plus long = aucun obstacle (hors portée)
const uint16_t US_NO_OBSTACLE_MM  = 4000;     // valeur retournée si hors portée
 
// --- Détection de pièce (hystérésis + confirmation) ---
const uint16_t PIECE_DETECT_MM   = 100;  // pièce présente si distance < 100 mm
const uint16_t PIECE_RELEASE_MM  = 120;  // pièce absente si distance > 120 mm (hystérésis)
const uint8_t  PIECE_CONFIRM_N   = 3;    // 3 mesures consécutives pour confirmer la présence
const uint8_t  PIECE_RELEASE_N   = 2;    // 2 mesures consécutives pour confirmer l'absence
 
// --- Contrôle qualité SIMULÉ (potentiomètre = "mesure" fictive 0..1023) ---
const uint16_t QUALITY_MIN = 300;  // conforme si QUALITY_MIN <= valeur <= QUALITY_MAX
const uint16_t QUALITY_MAX = 700;
 
// --- Temporisations de la machine à états ---
const uint32_t TRI_DURATION_MS        = 1500;   // durée du tri (pièce acheminée vers la sortie)
const uint32_t WAIT_PIECE_TIMEOUT_MS  = 15000;  // pas de pièce pendant ce délai => défaut "temps de cycle"
const uint32_t PIECE_STUCK_TIMEOUT_MS = 5000;   // pièce présente devant le capteur trop longtemps => défaut
const uint32_t RESET_DURATION_MS      = 500;    // durée de l'état RÉINITIALISATION
const uint8_t  SENSOR_FAIL_N          = 5;      // nb de mesures invalides consécutives => défaut capteur
 
// --- Servomoteur de tri (angles en degrés) ---
const uint8_t SERVO_NEUTRAL = 90;
const uint8_t SERVO_OK      = 45;
const uint8_t SERVO_NOK     = 135;
 
// --- Signalisation ---
const uint32_t BLINK_READY_MS = 500;  // clignotement lent LED convoyeur en état PRÊT
const uint32_t BLINK_ALARM_MS = 250;  // clignotement alarme en état DÉFAUT
const bool     BUZZER_ENABLED = true;
 
// --- Supervision série ---
const uint32_t STATUS_PERIOD_MS = 1000;
}  // namespace Cfg
 
