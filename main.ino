#define IR_USE_AVR_TIMER2 // 서보모터 충돌 방지(확인 필요)

#include "Component.h"
#include "TurtleMotion.h"
#include "TurtleModule.h"

constexpr byte RFID_0_START = 247;
constexpr byte RFID_1_START = 192;

/*
부품
서보모터 x 5, LED x 2, RFID, 조이스틱, 초음파센서

아두이노 핀 배치
D2(LED - 왼쪽 눈), (LED - 오른쪽 눈)
D3(서보모터 - 목) 
D4
D5(서보모터 - 오른쪽 앞다리)
D6(서보모터 - 오른쪽 뒷다리) 
D7(기울기센서)
D8(RFID RST)
D9
D10(RFID SDA)
D11(RFID MOSI)
D12(RFID MISO)
D13(RFID SCK)

A0(조이스틱 X)
A1(조이스틱 Y)
A2(조이스틱 SW)
A3(초음파 Trig)
A4(초음파 Echo)
A5(적외선 센서)

*/

void onRFIDRead(const byte* uid, byte uidSize);
void onJoyStickMove(uint16_t, uint16_t, bool);
void onDistanceDetected(uint16_t);
void onIRReceived(byte, bool);
void onTilt(bool);

LED mood(4);
Buzzer mouse(6);

TurtleHead tHead(3, 2);
TurtleLegs tLegs(5, 4);

SonarSensor eyeSensor(A3, A4, 500, onDistanceDetected);
RFID rfid(10, 8, 500, onRFIDRead);
JoyStick joyStick(A0, A1, A2, 0, onJoyStickMove);

IRController ir(A5, 200, onIRReceived);

BinarySensor tiltSensor(7, 200, onTilt, true);

MotionManager<TurtleHead> headMotion(tHead);
MotionManager<TurtleLegs> legsMotion(tLegs);

Initializable* initializables[] = {&tHead, &tLegs, &eyeSensor, &joyStick, &rfid, &ir, &tiltSensor};
// Initializable* initializables[] = {&tHead, &tLegs, &tiltSensor, &eyeSensor, &rfid, &joyStick, &ir};

// Updatable* updatables[] = {&headMotion, &legsMotion, &tiltSensor, &eyeSensor, &rfid, &joyStick, &ir, &tiltSensor};
Updatable* updatables[] = {&headMotion, &legsMotion, &eyeSensor, &joyStick, &rfid, &ir, &tiltSensor};

void onRFIDRead(const byte* uid, byte uidSize) {
  for(byte i=0; i < uidSize; i++) Serial.println(uid[i]);

  if(headMotion.isActing()) return;

  if(uid[0] == RFID_0_START) headMotion.book(MotionHandler::neckOut);
  if(uid[0] == RFID_1_START) headMotion.book(MotionHandler::neckOutFast);
}

void onJoyStickMove(uint16_t x, uint16_t y, bool sw) {
  mood.turnOn();
  Serial.println(x);
  // 0-1024 
  float fre = ((int)x + (int)y)/2.0f;
  mouse.play(fre, 1000);
}

void onDistanceDetected(uint16_t distance) {
  if(headMotion.isRunning() || distance == 999) return;

  if(distance < 20) {
    if(headMotion.getTarget().getNeck().getAngle() != MotionSetting::MAX_NECK_ANGLE) headMotion.attach(MotionHandler::neckIn);
  } else if(distance >= 20) {
    if(headMotion.getTarget().getNeck().getAngle() != MotionSetting::MIN_NECK_ANGLE) {
      headMotion.attach(MotionHandler::neckOut);
    }
  }  
}

void onIRReceived(byte value, bool x) {
  Serial.println(value);
}

void onTilt(bool isTilted) {
  // if(isTilted && !legsMotion.hasReversation()) legsMotion.book(MotionHandler::struggling);
}

void setup() {
  Serial.begin(9600);

  for(auto item: initializables) item->init();
  headMotion.attach(MotionHandler::neckOut);

  Serial.println(F("[setup] 로딩 완료"));
}

void loop() {
  unsigned long currentMillis = millis();
  for(auto item: updatables) item->update(currentMillis);
}
