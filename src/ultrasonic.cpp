#include "ultrasonic.h"
 
bool UltrasonicDriver::update(uint32_t nowUs, bool echo, Sample& out) {
  out = Sample();  // fresh=false par défaut
  switch (state_) {
    case WAIT_PERIOD:
      if ((uint32_t)(nowUs - tCycle_) >= Cfg::US_PERIOD_US) {
        tCycle_ = nowUs;
        if (echo) {                       // ECHO déjà haut avant le déclenchement: anomalie
          out.fresh = true; out.sensorOk = false;
          return false;
        }
        state_ = TRIG_HIGH; tStep_ = nowUs;
        return true;                      // début de l'impulsion TRIG
      }
      return false;
    case TRIG_HIGH:
      if ((uint32_t)(nowUs - tStep_) >= 10) {   // impulsion TRIG >= 10 µs
        state_ = WAIT_RISE; tStep_ = nowUs;
        return false;
      }
      return true;
    case WAIT_RISE:
      if (echo) { state_ = MEASURE; tRise_ = nowUs; }
      else if ((uint32_t)(nowUs - tStep_) > Cfg::US_RISE_TIMEOUT_US) {
        out.fresh = true; out.sensorOk = false;   // pas d'écho du tout: capteur débranché?
        state_ = WAIT_PERIOD;
      }
      return false;
    case MEASURE:
      if (!echo) {                         // fin de l'écho: durée = aller-retour du son
        uint32_t dur = (uint32_t)(nowUs - tRise_);
        out.fresh = true; out.sensorOk = true;
        out.distanceMm = (uint16_t)((dur * 10UL) / 58UL);  // ~58 µs par cm aller-retour
        state_ = WAIT_PERIOD;
      } else if ((uint32_t)(nowUs - tRise_) > Cfg::US_MAX_ECHO_US) {
        out.fresh = true; out.sensorOk = true;   // hors portée: pas d'obstacle
        out.distanceMm = Cfg::US_NO_OBSTACLE_MM;
        state_ = WAIT_PERIOD;
      }
      return false;
  }
  return false;
}
 
