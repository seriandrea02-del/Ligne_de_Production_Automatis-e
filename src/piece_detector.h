// piece_detector.h — Détection d'une pièce à partir des distances mesurées.
// Hystérésis + confirmation + réarmement: une pièce maintenue devant le capteur n'est comptée qu'UNE fois.
#pragma once
#include <stdint.h>
#include "config.h"
 
class PieceDetector {
 public:
  // Appeler avec chaque nouvelle mesure valide. Retourne true UNE fois à l'arrivée d'une pièce.
  bool onSample(uint16_t distMm) {
    bool arrived = false;
    if (!present_) {
      if (distMm < Cfg::PIECE_DETECT_MM) {
        if (++confirm_ >= Cfg::PIECE_CONFIRM_N) { present_ = true; arrived = true; confirm_ = 0; }
      } else confirm_ = 0;
    } else {
      if (distMm > Cfg::PIECE_RELEASE_MM) {
        if (++release_ >= Cfg::PIECE_RELEASE_N) { present_ = false; release_ = 0; }
      } else release_ = 0;
    }
    return arrived;
  }
  bool present() const { return present_; }
 
 private:
  bool present_ = false;
  uint8_t confirm_ = 0, release_ = 0;
};
