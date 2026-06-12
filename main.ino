#define IR_USE_AVR_TIMER2 // 서보모터와 TIMER1에서 충돌 방지를 위해 적외선 센서는 TIMER2사용

#include "Component.h"
#include "TurtleMotion.h"
#include "TurtleModule.h"

constexpr byte RFID_0_START = 192;
constexpr byte RFID_1_START = 183;
constexpr byte RFID_2_START = 233;

/*
부품
아두이노 우노, 빵판, 10K옴 저항, 220옴 저항, 릴레이 모듈, 배터리 AA 2구 x 2,
서보모터 x 3, LED x 2, RFID, 무드등
조이스틱, 초음파센서, IR 센서, 적외선 센서, RFID 모듈, 기울기 센서, 조도센서

아두이노 핀 배치
D2(LED - 왼쪽 눈), (LED - 오른쪽 눈)
D3(조이스틱 SW)
D4(Relay - 무드등)
D5(서보모터 - 오른쪽 앞다리) 180 시작!
D6(서보모터 - 왼쪽 앞다리) 
D7(기울기센서)
D8(RFID RST)
D9(서보모터 - 목)
D10(RFID SDA)
D11(RFID MOSI)
D12(RFID MISO)
D13(RFID SCK)

A0(조이스틱 X)
A1(조이스틱 Y)
A2(조도센서)
A3(초음파 Trig)
A4(초음파 Echo)
A5(적외선 센서)
*/

/*
거북이 로봇
기능
1. V 초음파 센서로 근거리 감지 되면 목 넣기r
2. V RFID로 목을 넣고 있을 때 뺴기
3. V 조이스틱으로 꼬리 만지면 좋아하기(LED)
4. V 기울기 센서로 뒤집히면 버둥거리기
5. 조도 센서로 강한 빛이 감지되면 일광욕하기(무드등 효과?)
6. V 적외선 수신기와 리모컨으로 각 모드를 강제 실행
*/

void onDistanceDetected(uint16_t);
void onRFIDRead(const byte* uid, byte uidSize);
void onJoyStickMove(uint16_t, uint16_t, bool);
void onTilt(bool);
void onLightDetected(uint16_t);
void onIRReceived(byte, bool);

Relay mood(4);
// Buzzer mouse(6);

TurtleHead tHead(9, 2);
TurtleLegs tLegs(5, 6);

SonarSensor eyeSensor(A3, A4, 500, onDistanceDetected);
RFID rfid(10, 8, 500, onRFIDRead);
JoyStick joyStick(A0, A1, 3, 0, onJoyStickMove);
BinarySensor tiltSensor(7, 200, onTilt, true);
PhotoResistor photoResistor(A2, 100, onLightDetected, true);
IRController ir(A5, 0, onIRReceived);

MotionManager<TurtleHead> headMotion(tHead);
MotionManager<TurtleLegs> legsMotion(tLegs);
MotionManager<Relay> moodMotion(mood);

Initializable* initializables[] = {&mood, &tHead, &tLegs, &eyeSensor, &rfid, &joyStick, &tiltSensor, &photoResistor, &ir};

Updatable* updatables[] = {&tHead, &headMotion, &legsMotion, &moodMotion, &eyeSensor, &rfid, &joyStick, &tiltSensor, &photoResistor, &ir};

void onDistanceDetected(uint16_t distance) {
  // Serial.println(distance);

  if(headMotion.isRunning() || distance == 999) return;

  if(distance < 20) {
    if(headMotion.getState() == NECK_OUT_END) headMotion.attach(MotionHandler::neckIn);
  } else if(distance >= 20) {
    if(headMotion.getState() == NECK_IN_END) headMotion.attach(MotionHandler::neckOut);
  }  
}

void onRFIDRead(const byte* uid, byte uidSize) {
  for(byte i=0; i < uidSize; i++) Serial.println(uid[i]);

  // if(headMotion.isActing()) return;

  if(headMotion.getState() != NECK_IN_END) return;

  if(uid[0] == RFID_0_START) {
    headMotion.setDelay(0);
    headMotion.attach(MotionHandler::neckOutFast);
  }
  if(uid[0] == RFID_1_START) {
    headMotion.setDelay(0);
    headMotion.attach(MotionHandler::neckOut);
  }
  if(uid[0] == RFID_2_START) {
    headMotion.setDelay(0);
    headMotion.attach(MotionHandler::angry);
  }
}

void onJoyStickMove(uint16_t x, uint16_t y, bool sw) {
  if(!legsMotion.isRunning() && !headMotion.isActing()) {
    headMotion.setDelay(0);
    headMotion.attach(MotionHandler::swimmingEyes);
    legsMotion.attach(MotionHandler::swimming);
  }
}
void onTilt(bool isTilted) {
  if(isTilted && !legsMotion.hasReversation()) legsMotion.book(MotionHandler::struggling);
}

void onLightDetected(uint16_t light) {
  if(light < PhotoResistor::TOO_DARK) {
    moodMotion.book(MotionHandler::moodLight);
  }

  if(light > PhotoResistor::TOO_BRIGHT) {
    if(!legsMotion.isRunning() && !headMotion.isRunning()) {
      legsMotion.attach(MotionHandler::sunbathe);
      if(headMotion.getState() == NECK_IN_END) {
        headMotion.setDelay(0);
        headMotion.attach(MotionHandler::neckOut);
      }
    }
  }
}

void onIRReceived(byte value, bool x) {
  if(!x) return;
  headMotion.setDelay(0);
  legsMotion.setDelay(0);

  switch(value) {
  case 1:
    legsMotion.attach(MotionHandler::swimming);
    break;
  case 2:
    headMotion.attach(MotionHandler::neckIn);
    break;
  case 3:
    headMotion.attach(MotionHandler::neckOutFast);
    break;
  case 4:
    headMotion.attach(MotionHandler::neckOut);
    break;
  case 5:
    if(mood.getState()) mood.turnOff();
    else mood.turnOn();
    break;
  case 6:
    if(tHead.getEyes().getState()) tHead.getEyes().turnOff();
    else tHead.getEyes().turnOn();
    break;

  }
}

void setup() {
  Serial.begin(9600);
  for(auto item: initializables) item->init();

  headMotion.attach(MotionHandler::neckOut);
  // legsMotion.attach(MotionHandler::swimming);

  tHead.getEyes().turnOn();

  Serial.println(F("[setup] 로딩 완료"));
  
}

void loop() {
  unsigned long currentMillis = millis();
  for(auto item: updatables) item->update(currentMillis);
}