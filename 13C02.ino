#include <Servo.h>

Servo servo;

float _SERVO_SPEED = 0.3; // 서보 속도    
int   TARGET_ANGLE = 90; // 목표 각도    
bool  RETURN_TO_ZERO = true; // 복귀를 위한 변수
float _RETURN_SPEED = 30.0;  // 복귀 할 때 속도


const int SERVO_PIN = 10;
const int START_ANGLE = 0;

void moveAtSpeed(int fromDeg, int toDeg, float dps) {
  if (dps <= 0) return;
  int dir = (toDeg >= fromDeg) ? 1 : -1;
  int steps = abs(toDeg - fromDeg); //총 이동 각도
  if (steps == 0) return;

  unsigned long total_ms = (unsigned long)(1000.0 * steps / dps); // 시간 계산 (이동각도/속도*1000)

  unsigned long per_step = max(1UL, total_ms / steps); // 1도 마다 지연시간 

  int angle = fromDeg;
  for (int i = 0; i < steps; i++) {
    angle += dir;
    servo.write(angle); // 목표 각도 부여
    delay(per_step); // 대기
  } 
}

void setup() {
  servo.attach(SERVO_PIN);
  servo.write(START_ANGLE); //0도에서 시작
  delay(1000);
}

void loop() {
  
  moveAtSpeed(START_ANGLE, TARGET_ANGLE, _SERVO_SPEED); // 목표한 각도까지 등속이동

  // (2) 0도로 빠르게 복귀
  if (RETURN_TO_ZERO) moveAtSpeed(TARGET_ANGLE, START_ANGLE, _RETURN_SPEED); // 끝나면 0도로 복귀하기

  while (1);
}
