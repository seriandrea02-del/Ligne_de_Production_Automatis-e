#include "event_format.h"
#include <stdio.h>
 
const char* const SUPERVISION_HEADER =
    "# FORMAT: EVT,ms,TYPE,... | STAT,ms,etat,total,conformes,non_conformes,defauts,defaut_actif,distance_mm,qualite";
 
size_t formatEvent(char* buf, size_t n, const Event& e, const Counters& c) {
  long ms = (long)e.ms;
  int r = 0;
  switch (e.type) {
    case EventType::BOOT:
      r = snprintf(buf, n, "EVT,%ld,DEMARRAGE,PRET_ATTENTE_START", ms); break;
    case EventType::STATE:
      r = snprintf(buf, n, "EVT,%ld,ETAT,%s,%s", ms, stateName((State)e.a), stateName((State)e.b)); break;
    case EventType::PIECE_DETECTED:
      r = snprintf(buf, n, "EVT,%ld,PIECE_DETECTEE,%u", ms, (unsigned)e.value); break;
    case EventType::PIECE_SORTED:
      r = snprintf(buf, n, "EVT,%ld,PIECE_TRIEE,%s,%u,%u,%u,%u", ms, qualityName((Quality)e.a), (unsigned)e.value,
                   (unsigned)c.total, (unsigned)c.ok, (unsigned)c.nok); break;
    case EventType::FAULT:
      r = snprintf(buf, n, "EVT,%ld,DEFAUT,%s,%s", ms, faultName((Fault)e.a), stateName((State)e.b)); break;
    case EventType::WARN:
      r = snprintf(buf, n, "EVT,%ld,AVERTISSEMENT,%s", ms, warnName((Warn)e.a)); break;
    case EventType::COUNTERS_RESET:
      r = snprintf(buf, n, "EVT,%ld,RAZ_COMPTEURS", ms); break;
  }
  return r < 0 ? 0 : (size_t)r;
}
 
size_t formatStatus(char* buf, size_t n, uint32_t ms, const ProductionFSM& f) {
  const Counters& c = f.counters();
  int r = snprintf(buf, n, "STAT,%ld,%s,%u,%u,%u,%u,%s,%u,%s", (long)ms, stateName(f.state()), (unsigned)c.total,
                   (unsigned)c.ok, (unsigned)c.nok, (unsigned)c.faults, faultName(f.fault()),
                   (unsigned)f.lastDistanceMm(), qualityName(f.lastQuality()));
  return r < 0 ? 0 : (size_t)r;
}
