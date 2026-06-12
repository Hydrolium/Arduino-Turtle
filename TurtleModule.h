#pragma once

#include "Component.h"


class TurtleHead: public Initializable, public Updatable {
  Servomotor360 neck;
  LED eyes;

public:
  TurtleHead(byte neckPin,  byte eyesPin): neck(neckPin), eyes(eyesPin){}

  void init() override {
    neck.init();
    eyes.init();
  }

  Servomotor360& getNeck() {
    return neck;
  }

  LED& getEyes() {
    return eyes;
  }

  void update(unsigned long currentMillis) override {
    neck.update(currentMillis);
  }
};

class TurtleLegs: public Initializable {
  Servomotor frontRightLeg;
  Servomotor frontLeftLeg;
public:
  TurtleLegs(byte frlPin, byte fllPin): frontRightLeg(frlPin, true), frontLeftLeg(fllPin, false) {}

  void init() override {
    frontRightLeg.init();
    frontLeftLeg.init();
  }

  Servomotor& getFrontRightLeg() {
    return frontRightLeg;
  }

  Servomotor& getFrontLeftLeg() {
    return frontLeftLeg;
  }
};