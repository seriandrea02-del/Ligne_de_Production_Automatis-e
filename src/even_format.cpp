// event_format.h — Formatage des lignes de supervision (CSV) dans un tampon.
// Partagé entre l'Arduino (Serial) et les tests sur PC => même format, vérifiable.
#pragma once
#include <stddef.h>
#include "fsm.h"
 
size_t formatEvent(char* buf, size_t n, const Event& e, const Counters& c);
size_t formatStatus(char* buf, size_t n, uint32_t ms, const ProductionFSM& f);
extern const char* const SUPERVISION_HEADER;
