#ifndef COMPONENT_H
#define COMPONENT_H

#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Stepper.h>
#include <IRremote.h>
#include <Ds1302.h>

class Initializable {
public:
  virtual void init() = 0;
  virtual ~Initializable() {}
};

class Updatable {
public:
  virtual void update(unsigned long currentMillis) = 0;
  virtual ~Updatable() {}
};

// 서보 모터
class Servomotor: public Initializable {
private:
  Servo servo;
  const byte servoPin;
  byte angle = 0;
public:
  static constexpr byte MAX_ANGLE = 180;

  Servomotor(byte pin): servoPin(pin) { }

  void init() override {
    servo.attach(servoPin);
    // servo.write(0);
  }

  void turn(byte a) {
    angle = constrain(a, 0, MAX_ANGLE);
    servo.write(a);
  }

  byte getAngle() const {
    return angle;
  }

};

// LED(on off)
class LED: public Initializable {
protected:
  const byte ledPin;
  bool state = LOW;
public:

  LED(byte pin): ledPin(pin) { }

  void init() override {
    pinMode(ledPin, OUTPUT);
  }

  bool getState() const {
    return state;
  }

  void turnOn() {
    state = HIGH;
    digitalWrite(ledPin, HIGH);
  }

  void turnOff() {
    state = LOW;
    digitalWrite(ledPin, LOW);
  }

};

// LED(밝기 조절)
class DimmableLED: public Initializable  {
protected:
  const byte ledPin;
  byte brightness;
public:

  DimmableLED(byte pin, byte bt=0): ledPin(pin), brightness(bt) { }

  void init() override {
    pinMode(ledPin, OUTPUT);
    analogWrite(ledPin, brightness);
  }

  void turnOn(byte bt) {
    brightness = bt;
    analogWrite(ledPin, bt);
  }

  void turnOff() {
    brightness = 0;
    analogWrite(ledPin, 0);
  }

  byte getBrightness() const {
    return brightness;
  }

};

// RGB LED
class RGBLED: public Initializable {
private:
  DimmableLED rLED;
  DimmableLED gLED;
  DimmableLED bLED;

public:

  RGBLED(byte rpin, byte gpin, byte bpin, byte rbt=0, byte gbt=0, byte bbt=0)
  : rLED(rpin, rbt), gLED(gpin, gbt), bLED(bpin, bbt) { }

  void init() override {
    rLED.init();
    gLED.init();
    bLED.init();
  }

  void turnOn(byte r, byte g, byte b) {
    rLED.turnOn(r);
    gLED.turnOn(g);
    bLED.turnOn(b);
  }

  void adjustBrightness(int16_t delta) {
    turnOn(
      constrain(rLED.getBrightness() + delta, 0, 255),
      constrain(gLED.getBrightness() + delta, 0, 255),
      constrain(bLED.getBrightness() + delta, 0, 255)
    );
  }

  byte getRBrightness() const {
    return rLED.getBrightness();
  }

  byte getGBrightness() const {
    return gLED.getBrightness();
  }

  byte getBBrightness() const {
    return bLED.getBrightness();
  }
};

// MP3 DFPlayer & speaker
class MP3Player: public Initializable {
private:
  SoftwareSerial softwareSerial;
  DFRobotDFPlayerMini dFPlayer;

  const byte volume; // 0~30

  bool connected = false;

public:
  // **DFPlayer tx엔 1K 저항 연결 필요
  MP3Player(byte rx, byte tx, byte v): softwareSerial(rx, tx), volume(v) {}

  void init() override {
    softwareSerial.begin(9600);

      if (!dFPlayer.begin(softwareSerial)) {
        Serial.println(F("[DFPlayer] Connect fail."));
        return;
      }

      connected = true;

      Serial.println(F("[DFPlayer] Connected"));
      dFPlayer.volume(volume);
  }

  void play(byte i) {
    dFPlayer.play(i);
  }

};

// 스텝모터
class StepMotor : public Initializable, public Updatable {
private:
  Stepper stepper;

  const uint16_t interval;
  const byte speed;
  unsigned long previousMillis = 0;

  int targetSteps = 0;
  bool stepDirection = true; // true: 정회전, false: 역회전

public:
  StepMotor(byte in1, byte in2, byte in3, byte in4, byte s, uint16_t i): stepper(2048, in1, in3, in2, in4), speed(s), interval(i) {}

  void init() override {
    stepper.setSpeed(speed);
  }

  void move(int steps) {
    targetSteps = abs(steps);
    stepDirection = steps > 0 ;
  }

  void update(unsigned long currentMillis) override {

    if(targetSteps <= 0) return;

    if (currentMillis - previousMillis < interval) return;
    previousMillis = currentMillis;

    stepper.step(stepDirection ? 1 : -1);
    targetSteps--;
  }
};


// RGC: Ds1302
class RealTimeClock: public Initializable {
private:
  Ds1302 rtc;
public:

  RealTimeClock(byte rstPin, byte clkPin, byte datPin): rtc(rstPin, clkPin, datPin) {}

  void init() override {
    rtc.init();
    // {
    //   char const *date = __DATE__;
    //   char const *time = __TIME__;

    //   char monthStr[4];
    //   int year, month, day, hour, minute, second;

    //   sscanf(date, "%s %d %d", monthStr, &day, &year);
    //   sscanf(time, "%d:%d:%d", &hour, &minute, &second);

    //   const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    //   for(int i = 0; i < 12; i++) {
    //     if(strcmp(monthStr, months[i]) == 0) {
    //       month = i + 1;
    //       break;
    //     }
    //   }

    //   Ds1302::DateTime dt = {
    //     .year = (uint8_t)(year % 100),
    //     .month = (uint8_t)month,
    //     .day = (uint8_t)day,
    //     .hour = (uint8_t)hour,
    //     .minute = (uint8_t)minute,
    //     .second = (uint8_t)second,
    //     .dow = 7 // 임시설정
    //   };

    //   rtc.setDateTime(&dt);
    // }
  }

  Ds1302::DateTime getDateTime() const {
    Ds1302::DateTime dt;
    rtc.getDateTime(&dt);
    return dt;
  }

};

class Buzzer: public Initializable {
private:
  const byte buzzerPin;

public:

  Buzzer(byte pin): buzzerPin(pin) { }

  void init() override {
    pinMode(buzzerPin, OUTPUT);
  }

  void play(uint16_t frequency, unsigned long time) {
    tone(buzzerPin, frequency, time);
  }
};

// 센서류

// 버튼, 기울기, RIR, 리드 스위치
class BinarySensor: public Initializable, public Updatable {
private:
  const byte sensorPin;
  const uint16_t interval;
  unsigned long previousMillis = 0;

  bool previousState = HIGH;

  const bool callHandlerAlways;

  void (*handler)(bool);

public:

  BinarySensor(byte pin, uint16_t i, void (*hd)(bool), bool cha = false): sensorPin(pin), interval(i), handler(hd), callHandlerAlways(cha) { }

  void init() override {
    pinMode(sensorPin, INPUT_PULLUP);
    previousState = digitalRead(sensorPin);
  }

  bool getState() const {
    return previousState;
  }

  void update(unsigned long currentMillis) override {

    if(handler == nullptr) return;

    if (currentMillis - previousMillis < interval) return;
    previousMillis = currentMillis;

    bool currentState = digitalRead(sensorPin);

    if(callHandlerAlways || previousState != currentState) handler(currentState);

    previousState = currentState;
  }

};

// 초음파 센서
class SonarSensor: public Initializable, public Updatable {

private:

  const byte trigPin;
  const byte echoPin;
  const uint16_t interval;
  unsigned long previousMillis = 0;

  byte stepStage = 0;
  unsigned long triggerStart;
  unsigned long echoStart;
  bool lastEchoState = LOW;

  uint16_t distance = 999;

  void (*handler)(uint16_t);

public:

  SonarSensor(byte trig, byte echo, unsigned i, void (*hd)(uint16_t)): trigPin(trig), echoPin(echo), interval(i), handler(hd) {}

  void init() override {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  }

  uint16_t getDistance() const {
    return distance;
  }

  void update(unsigned long currentMillis) override {

    // if (handler == nullptr) return;
    // if (millis() - previousMillis < interval) return;
    // previousMillis = millis();

    // digitalWrite(trigPin, LOW);
    // delayMicroseconds(2);
    // digitalWrite(trigPin, HIGH);
    // delayMicroseconds(10);
    // digitalWrite(trigPin, LOW);
    
    // // 2. Echo 핀이 HIGH를 유지한 시간(마이크로초) 측정
    // long duration = pulseIn(echoPin, HIGH);
    
    // // 3. 시간을 거리(cm)로 환산
    // long distance = duration * 0.034 / 2;
    
    // // 4. 시리얼 모니터에 결과 출력
    // Serial.print("Distance: ");
    // Serial.print(distance);
    // Serial.println(" cm");

    unsigned long currentMicros = micros();
    
    if (handler == nullptr) return;
    if (stepStage == 0) {
      if (currentMillis - previousMillis < interval) return;
      previousMillis = currentMillis;
      digitalWrite(trigPin, HIGH);
      triggerStart = currentMicros;
      stepStage = 1;
      return;
    }

    if(stepStage == 1) {
      if(currentMicros - triggerStart >= 10)  {
        digitalWrite(trigPin, LOW);
        stepStage = 2;
        lastEchoState = LOW;
      }
      return;
    }

    if(stepStage == 2) {
      bool currentEchoState = digitalRead(echoPin);
      if(lastEchoState == LOW && currentEchoState == HIGH) {
        echoStart = currentMicros;
      } else if(lastEchoState == HIGH && currentEchoState == LOW) {
        unsigned long duration = currentMicros - echoStart;
        distance = duration / 58;
        
        if(distance > 0 && distance < 400) handler(distance);
        else distance = 999;
        stepStage = 0;
      }

      lastEchoState = currentEchoState;

      if(currentMicros - triggerStart > 30000) {
        distance = 999;
        stepStage = 0;
      }
    }
  }
};


// RFID 센서
class RFID : public Initializable, public Updatable {
private:
  static constexpr byte MAX_UID_SIZE = 10;

  MFRC522 mfrc522;
  const uint16_t interval;
  unsigned long previousMillis = 0;

  byte lastUid[MAX_UID_SIZE];
  byte lastUidSize = 0;

  void (*handler)(const byte*, byte);

public:

  RFID(byte sdaPin, byte rstPin, uint16_t i, void (*hd)(const byte*, byte)): mfrc522(sdaPin, rstPin), interval(i), handler(hd) {
    for(byte i = 0; i < MAX_UID_SIZE; i++) lastUid[i] = 0;
  }

  void init() override {
    SPI.begin();
    mfrc522.PCD_Init();
    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max); 
  }

  const byte* getUid() const {
    return lastUid;
  }

  byte getUidSize() const {
    return lastUidSize;
  }

  void update(unsigned long currentMillis) override {

    if (handler == nullptr) return;

    if (currentMillis - previousMillis < interval) return;
    previousMillis = currentMillis;


    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    lastUidSize = mfrc522.uid.size;

    Serial.println(12321);

    if(lastUidSize > MAX_UID_SIZE) lastUidSize = MAX_UID_SIZE;

    for(byte i = 0; i < lastUidSize; i++) {
      lastUid[i] = mfrc522.uid.uidByte[i];
    }
    
    handler(lastUid, lastUidSize);

    mfrc522.PICC_HaltA(); 
  }
};

// 조이스틱
class JoyStick : public Initializable, public Updatable {
private:
  static constexpr byte IGNORE_LIMIT = 20;

  const byte xPin;
  const byte yPin;
  const byte swPin;

  const uint16_t interval;
  unsigned long previousMillis = 0;

  int lastX = 0;
  int lastY = 0;
  bool lastSw = false;

  const bool callHandlerAlways;
  void (*handler)(uint16_t, uint16_t, bool);

public:

  JoyStick(byte x, byte y, byte sw, uint16_t i, void (*hd)(uint16_t, uint16_t, bool), bool cha = false): xPin(x), yPin(y), swPin(sw), interval(i), handler(hd), callHandlerAlways(cha) {}

  void init() override {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
    pinMode(swPin, INPUT_PULLUP);

    lastX = analogRead(xPin);
    lastY = analogRead(yPin);
    lastSw = digitalRead(swPin) == LOW;
  }

  void update(unsigned long currentMillis) override {

    if(handler == nullptr) return;

    if (currentMillis - previousMillis < interval) return;
    previousMillis = currentMillis;

    int currentX = analogRead(xPin);
    int currentY = analogRead(yPin);
    bool currentSw = digitalRead(swPin) == LOW;

    if(callHandlerAlways) {
      handler(currentX, currentY, currentSw);
      return;
    }

    if(lastSw == currentSw && abs(currentX - lastX) <= IGNORE_LIMIT && abs(currentY - lastY) <= IGNORE_LIMIT) return;

    lastX = currentX;
    lastY = currentY;
    lastSw = currentSw;

    handler(currentX, currentY, currentSw);
  }
};

// 블루투스
class BlueTooth : public Initializable, public Updatable {
private:

  SoftwareSerial softwareSerial;

  const uint16_t interval;
  unsigned long previousMillis = 0;

  void (*handler)(SoftwareSerial&);

public:
  BlueTooth(byte rx, byte tx, uint16_t i, void (*hd)(SoftwareSerial&)): softwareSerial(rx, tx), interval(i), handler(hd) {}

  void init() override {
    softwareSerial.begin(9600);

    if(softwareSerial.available()) softwareSerial.read();
  }

  void update(unsigned long currentMillis) override {

    if(handler == nullptr) return;

    if (currentMillis - previousMillis < interval) return;
    previousMillis = currentMillis;

    if(softwareSerial.available()) handler(softwareSerial);
  }
};

// IR 센서
class IRController: public Initializable, public Updatable {
  const byte irPin;
  const uint16_t interval;
  unsigned long previousMillis = 0;

  unsigned long lastReceived = 0;
  bool lastDecoded = false;

  unsigned long lastSignalMillis = 0;
  const uint16_t timeoutDuration = 200;

  void (*handler)(byte, bool);

public:

  static constexpr unsigned long CODE_0 = 0XE916FF00;
  static constexpr unsigned long CODE_1 = 0XF30CFF00;
  static constexpr unsigned long CODE_2 = 0XE718FF00;
  static constexpr unsigned long CODE_3 = 0XA15EFF00;
  static constexpr unsigned long CODE_4 = 0XF708FF00;
  static constexpr unsigned long CODE_5 = 0XE31CFF00;
  static constexpr unsigned long CODE_6 = 0XA55AFF00;
  static constexpr unsigned long CODE_7 = 0XBD42FF00;
  static constexpr unsigned long CODE_8 = 0XAD52FF00;
  static constexpr unsigned long CODE_9 = 0XB54AFF00;

  static constexpr unsigned long CODE_EQ = 0XF609FF00;

  IRController(byte pin, uint16_t i, void (*hd)(byte, bool)): irPin(pin), interval(i), handler(hd) { }

  void init() override {
    IrReceiver.begin(irPin, ENABLE_LED_FEEDBACK);
  }

  unsigned long getReceived() const {
    return lastReceived;
  }

  byte toValue(unsigned long code) const {
    switch(code) {
      case IRController::CODE_0: return 0;
      case IRController::CODE_1: return 1;
      case IRController::CODE_2: return 2;
      case IRController::CODE_3: return 3;
      case IRController::CODE_4: return 4;
      case IRController::CODE_5: return 5;
      case IRController::CODE_6: return 6;
      case IRController::CODE_7: return 7;
      case IRController::CODE_8: return 8;
      case IRController::CODE_9: return 9;
      case IRController::CODE_EQ: return 10;
      default: return 255;
    }
  }

  void update(unsigned long currentMillis) override {

    if(handler == nullptr) return;

    if (currentMillis - previousMillis < interval) return;
    previousMillis = currentMillis;

    if(IrReceiver.decode()) {
      unsigned long received = IrReceiver.decodedIRData.decodedRawData;

      if(received != 0 && received != 0xFFFFFFFF) {
        lastReceived = received;
      }

      lastSignalMillis = currentMillis;

      if(!lastDecoded) {
        lastDecoded = true;
        byte value = toValue(lastReceived);
        if(value != 255) handler(value, true);
      }

      IrReceiver.resume();
    } else {
      if(lastDecoded && (currentMillis - lastSignalMillis >= timeoutDuration)) {
        lastDecoded = false;

        byte value = toValue(lastReceived);
        if(value != 255) handler(value, false);

        lastReceived = 0;
      }
    }

  }

};
#endif