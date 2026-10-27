// fsm.h — Machine à états de la ligne de production (logique pure, sans accès matériel).
// Entrées -> step() -> sorties. Aucune fonction bloquante: tout est piloté par l'heure passée en argument.
#pragma once
#include <stdint.h>
#include "config.h"
#include "piece_detector.h"
 
enum class State : uint8_t { ARRETE, PRET, PRODUCTION, TRI, DEFAUT, REINIT };
enum class Fault : uint8_t { NONE, ARRET_URGENCE, BLOCAGE_SIMULE, TEMPS_CYCLE, PIECE_BLOQUEE, CAPTEUR, COMPTEURS };
enum class Quality : uint8_t { NONE, CONFORME, NON_CONFORME };
 
// Entrées logiques (déjà anti-rebondies par le programme principal)
struct Inputs {
  bool startPressed = false;   // front montant consommé (un seul true par appui)
  bool stopPressed = false;
  bool resetPressed = false;
  bool faultSimActive = false; // niveau: bouton "blocage simulé" maintenu
  bool estopOpen = false;      // niveau: boucle d'arrêt d'urgence OUVERTE (= urgence)
  bool sampleFresh = false;    // true si le capteur a fourni une nouvelle mesure
  bool sensorOk = true;        // validité de la mesure
  uint16_t distanceMm = 0;
  uint16_t qualityRaw = 0;     // valeur 0..1023 (potentiomètre = mesure qualité simulée)
};
 
struct Outputs {
  bool conveyor = false;   // commande du convoyeur (LED verte = moteur simulé)
  bool ledReady = false;   // clignotement lent en état PRÊT
  bool ledOk = false, ledNok = false, ledAlarm = false, buzzer = false;
  uint8_t servoAngle = Cfg::SERVO_NEUTRAL;
};
 
struct Counters { uint16_t total = 0, ok = 0, nok = 0, faults = 0; };
 
enum class EventType : uint8_t { BOOT, STATE, PIECE_DETECTED, PIECE_SORTED, FAULT, WARN, COUNTERS_RESET };
enum class Warn : uint8_t {
  PIECE_IGNORED_STOPPED, PIECE_IGNORED_TRI, RESET_REFUSED, STOP_DEFERRED, PIECE_LOST, START_REFUSED
};
 
struct Event {
  uint32_t ms = 0;
  EventType type = EventType::BOOT;
  uint8_t a = 0, b = 0;   // paramètres selon le type (voir event_format.cpp)
  uint16_t value = 0;
};
 
typedef void (*EventSink)(const Event&);
 
class ProductionFSM {
 public:
  explicit ProductionFSM(EventSink sink = nullptr) : sink_(sink) {}
 
  void begin(uint32_t nowMs);
  void step(const Inputs& in, uint32_t nowMs);
 
  State state() const { return state_; }
  Fault fault() const { return fault_; }
  const Outputs& outputs() const { return out_; }
  const Counters& counters() const { return cnt_; }
  Quality lastQuality() const { return lastQuality_; }
  uint16_t lastDistanceMm() const { return lastDist_; }
  bool countersConsistent() const { return cnt_.total == (uint16_t)(cnt_.ok + cnt_.nok); }
 
#ifdef UNIT_TEST
  void debugCorruptCounters() { cnt_.total++; }
#endif
 
 private:
  void emit(EventType t, uint8_t a = 0, uint8_t b = 0, uint16_t v = 0);
  void goTo(State s, uint32_t now);
  void enterFault(Fault f, uint32_t now);
  void updateOutputs(uint32_t now);
 
  EventSink sink_;
  State state_ = State::PRET;
  Fault fault_ = Fault::NONE;
  Outputs out_;
  Counters cnt_;
  PieceDetector detector_;
  Quality lastQuality_ = Quality::NONE;
  uint16_t lastDist_ = 0, lastQualityRaw_ = 0;
  uint32_t tState_ = 0, tPieceSeen_ = 0, tNow_ = 0;
  uint8_t sensorFails_ = 0;
  bool stopPending_ = false;
};
 
const char* stateName(State s);
const char* faultName(Fault f);
const char* qualityName(Quality q);
const char* warnName(Warn w);
 
