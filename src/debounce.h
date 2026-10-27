// debouncer.h — Anti-rebond non bloquant (basé sur le temps, sans delay())
#pragma once
#include <stdint.h>
 
class Debouncer {
 public:
  explicit Debouncer(uint16_t stableMs = 30) : stableMs_(stableMs) {}
 
  // Initialise l'état stable sans générer de front (à appeler dans setup()).
  void reset(bool raw) { stable_ = candidate_ = raw; rising_ = false; tChange_ = 0; }
 
  // raw = true quand l'entrée est ACTIVE (bouton appuyé). À appeler à chaque tour de loop().
  void update(bool raw, uint32_t nowMs) {
    if (raw != candidate_) {            // changement brut: on relance le chronomètre
      candidate_ = raw;
      tChange_ = nowMs;
    } else if (candidate_ != stable_ && (uint32_t)(nowMs - tChange_) >= stableMs_) {
      stable_ = candidate_;             // le signal est resté stable assez longtemps
      if (stable_) rising_ = true;      // front montant (appui) mémorisé une seule fois
    }
  }
 
  bool active() const { return stable_; }
  // Retourne true UNE seule fois par appui (consomme le front).
  bool pressedEvent() { bool r = rising_; rising_ = false; return r; }
 
 private:
  uint16_t stableMs_;
  bool stable_ = false, candidate_ = false, rising_ = false;
  uint32_t tChange_ = 0;
};
