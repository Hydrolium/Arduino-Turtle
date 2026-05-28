#include <stdint.h>
#ifndef H
#define H

#include "Component.h"
#include "TurtleModule.h"

namespace MotionSetting {
  constexpr uint16_t MIN_NECK_ANGLE = 0;
  constexpr uint16_t MAX_NECK_ANGLE = 135;

  constexpr uint16_t DURATION_NECK_IN_NORMAL = 1000;
  constexpr uint16_t DELAY_NECK_IN = 1000; // 10000

  constexpr uint16_t DURATION_NECK_OUT_NORMAL = 1000;
  constexpr uint16_t DURATION_NECK_OUT_FAST = 1200;
  constexpr uint16_t DELAY_NECK_OUT = 500; // 5000

  constexpr uint16_t MIN_SWIMMING_ANGLE = 0;
  constexpr uint16_t MAX_SWIMMING_ANGLE = 180;

  constexpr uint16_t DURATION_SWIMMING = 3000;
  constexpr uint16_t DELAY_SWIMMING = 2000;
  
  constexpr uint16_t DURATION_STRUGGLING = 1000;
  constexpr uint16_t DELAY_STRUGGLING = 0;

  constexpr uint16_t MIN_STRUGLLING_ANGLE = 0;
  constexpr uint16_t MAX_STRUGLLING_ANGLE = 90;
}

/*
headmotion
NeckIn
NeckOut

legsmotion
SWIMMING
STRUGGLING
*/


template <typename Target>
class MotionManager: public Updatable {
private:
  unsigned long delayDuration = 0;
  unsigned long delayStart = 0;

  unsigned long motionStart = 0;

  Target &target;

  bool(*motion)(MotionManager&, unsigned long) = nullptr;
  bool(*nextMotion)(MotionManager&, unsigned long) = nullptr;

public:

  MotionManager(Target &t): target(t) {}

  void attach(bool(*mf)(MotionManager&, unsigned long)) {
    delayDuration = 0;
    delayStart = 0;
    motionStart = millis();
    motion = mf;
  }

  void detach() {
    motion = nullptr;
    delayDuration = 0;
  }

  void book(bool(*mf)(MotionManager&, unsigned long)) {
    nextMotion = mf;
  }

  void setDelay(unsigned long d) {
    delayStart = millis();
    delayDuration = d;
  }

  bool isActing() const {
    return motion != nullptr;
  }

  bool hasReversation() const {
    return nextMotion != nullptr;
  }

  bool isWaiting() const {
    return millis() - delayStart < delayDuration;
  }

  bool isRunning() const {
    return isActing() || hasReversation() || isWaiting();
  }

  Target& getTarget() const {
    return target;
  }

  void update(unsigned long currentMillis) override {

    if(isWaiting()) return;

    if(!isActing()) {
      if(nextMotion != nullptr) {
        attach(nextMotion);
        nextMotion = nullptr;
      }
      return;
    }

    if(motion(*this, millis() - motionStart)) motion = nullptr;
    
  }
};

namespace MotionHandler {

  inline float easing(float x) {
    return x * x *(3.0f - 2.0f*x);
  }

  inline float smooth(float x, float maxX) {
    return easing(x/maxX);
  }

  inline float smoothRev(float x, float maxX) {
    return 1 - easing(x/maxX);
  }

  inline float smoothGoBack(float x, float maxX) {
    x /= maxX;
    return easing((x < 0.5f) ? x * 2.0f : (1.0f - x) * 2.0f);
  }

  inline float setWithRatio(float ratio, float minX, float maxX) {
    return minX + ratio * (maxX - minX);
  }

  bool neckIn(MotionManager<TurtleHead> &th, unsigned long time) {
    TurtleHead &head = th.getTarget();

    if (time >= MotionSetting::DURATION_NECK_IN_NORMAL) {
      head.getNeck().turn(MotionSetting::MAX_NECK_ANGLE);
      head.getEyes().turnOn();
      th.setDelay(MotionSetting::DELAY_NECK_IN);
      return true;
    }

    head.getNeck().turn(
      setWithRatio(smooth(time, MotionSetting::DURATION_NECK_IN_NORMAL), MotionSetting::MIN_NECK_ANGLE, MotionSetting::MAX_NECK_ANGLE)
    );

    return false;
  }

  bool neckOut(MotionManager<TurtleHead> &th, unsigned long time) {
    TurtleHead &head = th.getTarget();

    if (time >= MotionSetting::DURATION_NECK_OUT_NORMAL) {
      head.getNeck().turn(MotionSetting::MIN_NECK_ANGLE);
      head.getEyes().turnOff();
      th.setDelay(MotionSetting::DELAY_NECK_OUT);
      return true;
    }
    
    head.getNeck().turn(
      setWithRatio(smoothRev(time, MotionSetting::DURATION_NECK_OUT_NORMAL), MotionSetting::MIN_NECK_ANGLE, MotionSetting::MAX_NECK_ANGLE)
    );

    return false;
  }

  bool neckOutFast(MotionManager<TurtleHead> &th, unsigned long time) {

    TurtleHead &head = th.getTarget();

    if (time >= MotionSetting::DURATION_NECK_OUT_FAST) {
      head.getNeck().turn(MotionSetting::MIN_NECK_ANGLE);
      head.getEyes().turnOff();
      th.setDelay(MotionSetting::DELAY_NECK_OUT);
      return true;
    }

    head.getNeck().turn(
      setWithRatio(smoothRev(time, MotionSetting::DURATION_NECK_OUT_FAST), MotionSetting::MIN_NECK_ANGLE, MotionSetting::MAX_NECK_ANGLE)
    );

    return false;
  }

  bool swimming(MotionManager<TurtleLegs> &tl, unsigned long time) {
    
    TurtleLegs &legs = tl.getTarget();
    
    if(time >= MotionSetting::DURATION_SWIMMING) {
      tl.setDelay(MotionSetting::DELAY_SWIMMING);
      legs.getFrontRightLeg().turn(MotionSetting::MIN_SWIMMING_ANGLE);
      legs.getFrontLeftLeg().turn(MotionSetting::MIN_SWIMMING_ANGLE);
      return true;
    }
    
    byte nextAngle = setWithRatio(smoothGoBack(time, MotionSetting::DURATION_SWIMMING), MotionSetting::MIN_SWIMMING_ANGLE, MotionSetting::MAX_SWIMMING_ANGLE);

    legs.getFrontRightLeg().turn(nextAngle);
    legs.getFrontLeftLeg().turn(nextAngle);

    return false;
  }

  bool struggling(MotionManager<TurtleLegs> &tl, unsigned long time) {
    
    TurtleLegs &legs = tl.getTarget();
    
    if(time >= MotionSetting::DURATION_STRUGGLING) {
      tl.setDelay(MotionSetting::DELAY_STRUGGLING);
      legs.getFrontRightLeg().turn(MotionSetting::MIN_STRUGLLING_ANGLE);
      legs.getFrontLeftLeg().turn(MotionSetting::MIN_STRUGLLING_ANGLE);
      return true;
    }

    byte nextAngle = setWithRatio(smoothGoBack(time, MotionSetting::DURATION_STRUGGLING), MotionSetting::MIN_STRUGLLING_ANGLE, MotionSetting::MAX_STRUGLLING_ANGLE);

    legs.getFrontRightLeg().turn(nextAngle);
    legs.getFrontLeftLeg().turn(nextAngle);

    return false;
  }
}

#endif