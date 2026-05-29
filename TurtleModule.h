#pragma once

#include "Component.h"

class TurtleHead: public Initializable {
  Servomotor neck;
  LED eyes;
public:
  TurtleHead(byte neckPin,  byte eyesPin): neck(neckPin), eyes(eyesPin){}

  void init() override {
    neck.init();
    eyes.init();
  }

  Servomotor& getNeck() {
    return neck;
  }

  LED& getEyes() {
    return eyes;
  }
};

class TurtleLegs: public Initializable {
  Servomotor frontRightLeg;
  Servomotor frontLeftLeg;
public:
  TurtleLegs(byte frlPin, byte fllPin): frontRightLeg(frlPin), frontLeftLeg(fllPin) {}

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