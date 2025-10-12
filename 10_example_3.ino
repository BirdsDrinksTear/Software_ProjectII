#include <Servo.h>

#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

#define SND_VEL 346.0
#define PULSE_DURATION 10
#define TIMEOUT 20000UL           
#define SCALE (0.001 * 0.5 * SND_VEL) 

#define _DIST_MIN 100.0
#define _DIST_MAX 300.0


#define DOWN_ANGLE   0
#define UP_ANGLE     90
#define MOVING_TIME  3000UL // 3초동안 부드럽게 이동

// 히스테리시스: 접근/이탈 문턱 분리(깜빡임 방지)
#define NEAR_ENTER_MM 200.0     
#define NEAR_EXIT_MM  220.0   

Servo barrier;
bool vehiclePresent = false;  // 차량 감지 상태

unsigned long moveStartTime = 0;  // 이동 시작 시각
bool isMoving = false;
int  startAngle = DOWN_ANGLE;
int  stopAngle  = DOWN_ANGLE;

// 1은 Sigmoid, 2는 Cosine 설정ㅇ
int easingMode = 1; // 기본을 Sigmoid로

// 초음파 거리 측정(mm)
float measureDistanceMM() {
  digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(PULSE_DURATION);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long dur = pulseIn(PIN_ECHO, HIGH, TIMEOUT);
  return (float)dur * SCALE;  // mm로
}

//Sigmoid 함수
float easeSigmoid01(float t) {
  
  const float k = 10.0f;
  auto L = [&](float x){ return 1.0f / (1.0f + expf(-k*(x - 0.5f))); };
  float y0 = L(0.0f), y1 = L(1.0f);
  return (L(t) - y0) / (y1 - y0); //0~1로 정규화 시키기
}

//Cosine 함수
float easeCosine01(float t) {
  return 0.5f - 0.5f * cosf(PI * t); // 부드럽게 가속, 감속 시키기
}

// 기본 함수 선택
float ease01(float t) {
  if (t <= 0) return 0;
  if (t >= 1) return 1;
  return (easingMode == 1) ? easeSigmoid01(t) : easeCosine01(t);
}

// 차단기 이동 시작
void startMotion(int toAngle) {
  isMoving = true;
  moveStartTime = millis();
  startAngle = barrier.read();
  stopAngle = toAngle;
}


void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  Serial.begin(57600);

  barrier.attach(PIN_SERVO);
  barrier.write(DOWN_ANGLE); 

  Serial.println("Parking Barrier (1=Sigmoid, 2=Cosine)");
}

void loop() {
  // 차량 감지
  float d = measureDistanceMM();
  if (d <= 0 || d > _DIST_MAX) d = _DIST_MAX + 10; // 실패/과원거리 → 무효 취급

  bool prev = vehiclePresent;
  if (!vehiclePresent && d <= NEAR_ENTER_MM) vehiclePresent = true; // 접근
  if ( vehiclePresent && d >= NEAR_EXIT_MM ) vehiclePresent = false;  // 이탈

  // 상태 변화 감지할 경우 이동
  if (vehiclePresent != prev) {
    startMotion(vehiclePresent ? UP_ANGLE : DOWN_ANGLE);
    Serial.println(vehiclePresent ? "Car Detected → OPEN" : "Car Left → CLOSE");
  }

  //서보 모터를 부드럽게
  if (isMoving) {
    unsigned long progress = millis() - moveStartTime;
    if (progress <= MOVING_TIME) {
      float t = (float)progress / (float)MOVING_TIME; // 0~1
      float u = ease01(t); // 이징 결과
      int angle = (int)round(startAngle + (stopAngle - startAngle) * u);
      barrier.write(angle);
    } else {
      barrier.write(stopAngle);
      isMoving = false;
    }
  }

  //시리얼 모니터로 이징 모드 전환
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '1') { easingMode = 1; Serial.println("Mode: Sigmoid"); }
    if (c == '2') { easingMode = 2; Serial.println("Mode: Cosine"); }
  }

  //상태 출력
  Serial.print("Dist(mm)="); Serial.print(d);
  Serial.print(" | State="); Serial.print(vehiclePresent ? "ON" : "OFF");
  Serial.print(" | Angle="); Serial.print(barrier.read());
  Serial.print(" | Easing="); Serial.println(easingMode==1 ? "SIG" : "COS");

  delay(50); 
}
