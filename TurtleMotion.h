#pragma once

#include <stdint.h>
#include "Component.h"
#include "TurtleModule.h"

namespace MotionSetting {

  constexpr uint16_t DURATION_NECK_IN_NORMAL = 4500;
  constexpr uint16_t SPEED_NECK_IN = 30;
  constexpr uint16_t DELAY_NECK_IN = 10000;

  constexpr uint16_t DURATION_NECK_OUT_NORMAL = 3000;
  constexpr uint16_t DURATION_NECK_OUT_FAST = 2000;
  constexpr uint16_t SPEED_NECK_OUT_NORMAL = 145;
  constexpr uint16_t SPEED_NECK_OUT_FAST = 170;

  constexpr uint16_t DELAY_NECK_OUT = 5000;

  constexpr uint16_t MIN_SWIMMING_ANGLE = 0;
  constexpr uint16_t MAX_SWIMMING_ANGLE = 60;

  constexpr uint16_t DURATION_SWIMMING = 4000;
  constexpr uint16_t INTERVAL_SWIMMING_EYE = 500;
  constexpr uint16_t DELAY_SWIMMING = 0;

  constexpr uint16_t DURATION_ANGRY = 2000;
  constexpr uint16_t INTERVAL_ANGRY = 50;
  constexpr uint16_t DELAY_ANGRY = 0;
  
  constexpr uint16_t DURATION_STRUGGLING = 1000;
  constexpr uint16_t DELAY_STRUGGLING = 0;

  constexpr uint16_t MIN_STRUGLLING_ANGLE = 0;
  constexpr uint16_t MAX_STRUGLLING_ANGLE = 70;

  constexpr uint16_t DURATION_SUNBATH = 10000;
  constexpr uint16_t DELAY_SUNBATH = 5000;

  constexpr uint16_t MIN_SUNBATH_ANGLE = 0;
  constexpr uint16_t MAX_SUNBATH_ANGLE = 50;

  constexpr uint16_t DURATION_MOOD = 1000;
  constexpr uint16_t DELAY_MOOD = 0;
}

/*
headmotion
NeckIn
NeckOut

legsmotion
SWIMMING
STRUGGLING
*/
enum MotionState {NORMAL, NECK_IN, NECK_IN_END, NECK_OUT, NECK_OUT_END, SUNBATH, SUNBATH_END};

template <typename Target>
class MotionManager: public Updatable {
private:
  unsigned long delayDuration = 0;
  unsigned long delayStart = 0;

  unsigned long motionStart = 0;

  MotionState state = NORMAL;

  Target &target;

  bool(*motion)(MotionManager&, unsigned long) = nullptr;
  bool(*nextMotion)(MotionManager&, unsigned long) = nullptr;

public:

  MotionManager(Target &t): target(t) {}

  MotionState getState() const {
    return state;
  }

  void setState(MotionState s) {
    state = s;
  }

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
      head.getEyes().turnOn();
      head.getNeck().stop();
      th.setDelay(MotionSetting::DELAY_NECK_IN);
      th.setState(NECK_IN_END);

      return true;
    }

    th.setState(NECK_IN);
    head.getNeck().turn(MotionSetting::SPEED_NECK_IN);

    return false;
  }

  bool neckOut(MotionManager<TurtleHead> &th, unsigned long time) {
    TurtleHead &head = th.getTarget();

    if (time >= MotionSetting::DURATION_NECK_OUT_NORMAL) {
      head.getEyes().turnOff();
      head.getNeck().stop();
      th.setDelay(MotionSetting::DELAY_NECK_OUT);
      th.setState(NECK_OUT_END);

      return true;
    }
    
    th.setState(NECK_OUT);
    head.getNeck().turn(MotionSetting::SPEED_NECK_OUT_NORMAL);

    return false;
  }

  bool neckOutFast(MotionManager<TurtleHead> &th, unsigned long time) {

    TurtleHead &head = th.getTarget();

    if (time >= MotionSetting::DURATION_NECK_OUT_FAST) {
      head.getEyes().turnOff();
      head.getNeck().stop();

      th.setDelay(MotionSetting::DELAY_NECK_OUT);
      th.setState(NECK_OUT_END);
      return true;
    }

    th.setState(NECK_OUT);
    head.getNeck().turn(MotionSetting::SPEED_NECK_OUT_FAST);

    return false;
  }

  bool angry(MotionManager<TurtleHead> &th, unsigned long time) {

    static unsigned long lastTime = 0;

    TurtleHead &head = th.getTarget();
    LED& eyes = head.getEyes();
    
    if(time >= MotionSetting::DURATION_ANGRY) {
      th.setDelay(MotionSetting::DELAY_ANGRY);
      eyes.turnOff();

      return true;
    }

    if(time - lastTime > MotionSetting::INTERVAL_ANGRY) {
      if(eyes.getState()) eyes.turnOff();
      else eyes.turnOn();

      lastTime = time;
    }

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

  bool swimmingEyes(MotionManager<TurtleHead> &th, unsigned long time) {

    static unsigned long lastTime = 0;

    TurtleHead &head = th.getTarget();
    LED& eyes = head.getEyes();
    
    if(time >= MotionSetting::DURATION_SWIMMING) {
      th.setDelay(MotionSetting::DELAY_SWIMMING);
      eyes.turnOff();

      return true;
    }

    if(time - lastTime > MotionSetting::INTERVAL_SWIMMING_EYE) {
      if(eyes.getState()) eyes.turnOff();
      else eyes.turnOn();

      lastTime = time;
    }

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

  bool sunbathe(MotionManager<TurtleLegs> &tl, unsigned long time) {
    TurtleLegs &legs = tl.getTarget();
    
    if(time >= MotionSetting::DURATION_SUNBATH) {
      tl.setDelay(MotionSetting::DELAY_SUNBATH);
      legs.getFrontRightLeg().turn(MotionSetting::MIN_SUNBATH_ANGLE);
      legs.getFrontLeftLeg().turn(MotionSetting::MIN_SUNBATH_ANGLE);
      return true;
    }

    unsigned long inOutDuration = MotionSetting::DURATION_SUNBATH / 10;

    if(time <= inOutDuration) {
      byte nextAngle = setWithRatio(smooth(time, inOutDuration), MotionSetting::MIN_SUNBATH_ANGLE, MotionSetting::MAX_SUNBATH_ANGLE);
      legs.getFrontRightLeg().turn(nextAngle);
      legs.getFrontLeftLeg().turn(nextAngle);
    } else if(time <= MotionSetting::DURATION_SUNBATH - inOutDuration) {
      legs.getFrontRightLeg().turn(MotionSetting::MAX_SUNBATH_ANGLE);
      legs.getFrontLeftLeg().turn(MotionSetting::MAX_SUNBATH_ANGLE);
    } else {
      unsigned long elapsed = time - (MotionSetting::DURATION_SUNBATH - inOutDuration);
      byte nextAngle = setWithRatio(smoothRev(elapsed, inOutDuration), MotionSetting::MIN_SUNBATH_ANGLE, MotionSetting::MAX_SUNBATH_ANGLE);
      legs.getFrontRightLeg().turn(nextAngle);
      legs.getFrontLeftLeg().turn(nextAngle);
    }

    return false;
  }

  bool moodLight(MotionManager<Relay> &tm, unsigned long time) {
    
    Relay &mood = tm.getTarget();
    
    if(time >= MotionSetting::DURATION_STRUGGLING) {
      tm.setDelay(MotionSetting::DELAY_MOOD);
      mood.turnOff();
      return true;
    }

    mood.turnOn();

    return false;
  }
}