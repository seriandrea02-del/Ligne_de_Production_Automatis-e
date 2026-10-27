// fsm.cpp — Implémentation de la machine à états (voir README pour le diagramme).
#include "fsm.h"
 
void ProductionFSM::emit(EventType t, uint8_t a, uint8_t b, uint16_t v) {
  if (!sink_) return;
  Event e; e.ms = tNow_; e.type = t; e.a = a; e.b = b; e.value = v;
  sink_(e);
}
 
void ProductionFSM::begin(uint32_t nowMs) {
  tNow_ = nowMs; tState_ = nowMs;
  state_ = State::PRET;               // F1: jamais de démarrage automatique après mise sous tension
  emit(EventType::BOOT);
  updateOutputs(nowMs);
}
 
void ProductionFSM::goTo(State s, uint32_t now) {
  State old = state_;
  state_ = s; tState_ = now;
  if (s == State::PRODUCTION) stopPending_ = false;
  if (s == State::REINIT) sensorFails_ = 0;
  emit(EventType::STATE, (uint8_t)old, (uint8_t)s);
}
 
void ProductionFSM::enterFault(Fault f, uint32_t now) {
  if (state_ == State::TRI) emit(EventType::WARN, (uint8_t)Warn::PIECE_LOST);  // pièce non comptée
  fault_ = f;
  cnt_.faults++;
  stopPending_ = false;
  emit(EventType::FAULT, (uint8_t)f, (uint8_t)state_);
  goTo(State::DEFAUT, now);
}
 
void ProductionFSM::step(const Inputs& in, uint32_t now) {
  tNow_ = now;
 
  // 1) Exploitation de la mesure du capteur (toujours suivie, même à l'arrêt)
  bool pieceArrived = false;
  if (in.sampleFresh) {
    if (in.sensorOk) {
      sensorFails_ = 0;
      lastDist_ = in.distanceMm;
      if (detector_.onSample(in.distanceMm)) {
        pieceArrived = true;
        tPieceSeen_ = now;
        emit(EventType::PIECE_DETECTED, 0, 0, in.distanceMm);
      }
    } else if (sensorFails_ < 255) {
      sensorFails_++;
    }
  }
 
  // 2) Défauts prioritaires, valables dans tous les états sauf DEFAUT
  if (state_ != State::DEFAUT) {
    if (in.estopOpen) { enterFault(Fault::ARRET_URGENCE, now); updateOutputs(now); return; }
    if (sensorFails_ >= Cfg::SENSOR_FAIL_N) { enterFault(Fault::CAPTEUR, now); updateOutputs(now); return; }
  }
 
  // 3) Comportement propre à chaque état
  switch (state_) {
    case State::ARRETE:
    case State::PRET:
      if (pieceArrived) emit(EventType::WARN, (uint8_t)Warn::PIECE_IGNORED_STOPPED);
      if (in.startPressed) goTo(State::PRODUCTION, now);
      else if (in.resetPressed) { cnt_ = Counters(); emit(EventType::COUNTERS_RESET); }
      break;
 
    case State::PRODUCTION:
      if (in.faultSimActive) enterFault(Fault::BLOCAGE_SIMULE, now);
      else if (in.stopPressed) goTo(State::ARRETE, now);
      else if (detector_.present() && (uint32_t)(now - tPieceSeen_) >= Cfg::PIECE_STUCK_TIMEOUT_MS)
        enterFault(Fault::PIECE_BLOQUEE, now);
      else if (pieceArrived) {
        lastQualityRaw_ = in.qualityRaw;
        lastQuality_ = (in.qualityRaw >= Cfg::QUALITY_MIN && in.qualityRaw <= Cfg::QUALITY_MAX)
                           ? Quality::CONFORME : Quality::NON_CONFORME;
        goTo(State::TRI, now);
      }
      else if ((uint32_t)(now - tState_) >= Cfg::WAIT_PIECE_TIMEOUT_MS)
        enterFault(Fault::TEMPS_CYCLE, now);
      break;
 
    case State::TRI:
      if (in.faultSimActive) { enterFault(Fault::BLOCAGE_SIMULE, now); break; }
      if (detector_.present() && (uint32_t)(now - tPieceSeen_) >= Cfg::PIECE_STUCK_TIMEOUT_MS) {
        enterFault(Fault::PIECE_BLOQUEE, now); break;
      }
      if (in.stopPressed && !stopPending_) { stopPending_ = true; emit(EventType::WARN, (uint8_t)Warn::STOP_DEFERRED); }
      if (pieceArrived) emit(EventType::WARN, (uint8_t)Warn::PIECE_IGNORED_TRI);
      if ((uint32_t)(now - tState_) >= Cfg::TRI_DURATION_MS) {
        // La pièce est comptée ICI, une seule fois, à la fin du tri
        cnt_.total++;
        if (lastQuality_ == Quality::CONFORME) cnt_.ok++; else cnt_.nok++;
        emit(EventType::PIECE_SORTED, (uint8_t)lastQuality_, 0, lastQualityRaw_);
        if (!countersConsistent()) enterFault(Fault::COMPTEURS, now);
        else if (stopPending_) goTo(State::ARRETE, now);
        else goTo(State::PRODUCTION, now);
      }
      break;
 
    case State::DEFAUT:
      if (in.startPressed) emit(EventType::WARN, (uint8_t)Warn::START_REFUSED);
      if (in.resetPressed) {
        // On refuse la réinitialisation tant que la cause est encore présente
        if (in.estopOpen || in.faultSimActive) emit(EventType::WARN, (uint8_t)Warn::RESET_REFUSED);
        else goTo(State::REINIT, now);
      }
      break;
 
    case State::REINIT:
      if (in.startPressed) emit(EventType::WARN, (uint8_t)Warn::START_REFUSED);
      if ((uint32_t)(now - tState_) >= Cfg::RESET_DURATION_MS) {
        fault_ = Fault::NONE;
        goTo(State::PRET, now);
      }
      break;
  }
  updateOutputs(now);
}
 
void ProductionFSM::updateOutputs(uint32_t now) {
  Outputs o;  // tout à zéro par défaut (sécurité: sortie inactive sauf si l'état l'exige)
  switch (state_) {
    case State::ARRETE: break;
    case State::PRET:
      o.ledReady = (((now - tState_) / Cfg::BLINK_READY_MS) % 2) == 0;
      break;
    case State::PRODUCTION:
      o.conveyor = true;
      break;
    case State::TRI:
      o.conveyor = true;
      o.ledOk = (lastQuality_ == Quality::CONFORME);
      o.ledNok = (lastQuality_ == Quality::NON_CONFORME);
      o.servoAngle = o.ledOk ? Cfg::SERVO_OK : Cfg::SERVO_NOK;
      break;
    case State::DEFAUT: {
      bool on = (((now - tState_) / Cfg::BLINK_ALARM_MS) % 2) == 0;
      o.ledAlarm = on;
      o.buzzer = on && Cfg::BUZZER_ENABLED;
      break;
    }
    case State::REINIT:
      o.ledAlarm = true;
      break;
  }
  out_ = o;
}
 
const char* stateName(State s) {
  switch (s) {
    case State::ARRETE: return "ARRETE";
    case State::PRET: return "PRET";
    case State::PRODUCTION: return "PRODUCTION";
    case State::TRI: return "TRI";
    case State::DEFAUT: return "DEFAUT";
    case State::REINIT: return "REINIT";
  }
  return "?";
}
const char* faultName(Fault f) {
  switch (f) {
    case Fault::NONE: return "AUCUN";
    case Fault::ARRET_URGENCE: return "ARRET_URGENCE";
    case Fault::BLOCAGE_SIMULE: return "BLOCAGE_SIMULE";
    case Fault::TEMPS_CYCLE: return "TEMPS_CYCLE";
    case Fault::PIECE_BLOQUEE: return "PIECE_BLOQUEE";
    case Fault::CAPTEUR: return "CAPTEUR";
    case Fault::COMPTEURS: return "COMPTEURS";
  }
  return "?";
}
const char* qualityName(Quality q) {
  switch (q) {
    case Quality::NONE: return "AUCUNE";
    case Quality::CONFORME: return "CONFORME";
    case Quality::NON_CONFORME: return "NON_CONFORME";
  }
  return "?";
}
const char* warnName(Warn w) {
  switch (w) {
    case Warn::PIECE_IGNORED_STOPPED: return "PIECE_IGNOREE_CONVOYEUR_ARRETE";
    case Warn::PIECE_IGNORED_TRI: return "PIECE_IGNOREE_PENDANT_TRI";
    case Warn::RESET_REFUSED: return "RESET_REFUSE_CAUSE_PRESENTE";
    case Warn::STOP_DEFERRED: return "ARRET_DIFFERE_FIN_DE_TRI";
    case Warn::PIECE_LOST: return "PIECE_NON_COMPTEE_DEFAUT_PENDANT_TRI";
    case Warn::START_REFUSED: return "START_REFUSE_DEFAUT_EN_COURS";
  }
  return "?";
}
