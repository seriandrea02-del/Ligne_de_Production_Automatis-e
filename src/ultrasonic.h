// ultrasonic.h — Pilote HC-SR04 NON BLOQUANT, indépendant du matériel.
// La logique reçoit l'heure (µs) et le niveau de ECHO, et retourne le niveau à appliquer sur TRIG.
// => testable sur PC avec un écho simulé.
#pragma once
#include <stdint.h>
#include "config.h"
 
class UltrasonicDriver {
 public:
  struct Sample {
    bool fresh = false;        // true si une nouvelle mesure est disponible
    bool sensorOk = true;      // false : capteur absent (ECHO ne monte pas) ou ECHO bloqué à l'état haut
    uint16_t distanceMm = 0;   // distance en mm (US_NO_OBSTACLE_MM si hors portée)
  };
 
  // Retourne le niveau à écrire sur TRIG. 'out' est rempli (fresh=false si rien de nouveau).
  bool update(uint32_t nowUs, bool echo, Sample& out);
 
 private:
  enum State : uint8_t { WAIT_PERIOD, TRIG_HIGH, WAIT_RISE, MEASURE };
  State state_ = WAIT_PERIOD;
  uint32_t tCycle_ = 0, tStep_ = 0, tRise_ = 0;
};
