// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 100      // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // coefficent to convert duration to distance

unsigned long last_sampling_time;   // unit: msec
float distance;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW); 
  Serial.begin(57600);
}

void loop() { 
  // wait until next sampling time
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance

  // 범위 바깥 값 보정
  if ((distance == 0.0) || (distance > _DIST_MAX)) {
    distance = _DIST_MAX + 10.0;   
  } else if (distance < _DIST_MIN) {
    distance = _DIST_MIN - 10.0;    
  }

  int pwm; // 밝기 조절을 위한 int 변수 생성
  if (distance <= 100 || distance >= 300) { // 100 이하 300 이상에서 
    pwm = 255; // 꺼짐
  } 
  else if (distance <= 200) { //100 초과 200이하에서
    float ratio = (distance - 100.0) / 100.0; // 비례식을 위해 ratio를 float변수로 만들고, distance가 늘어날수록 ratio가 커짐, 이때 100을 뺴야 함(100초과 거리니까)
    pwm = 255 - (int)(255 * ratio); // 200에 가까워질수로 ratio는 1에 가까워지고, pwm은 distance가 200에 가까워질수록 0에 가까워짐
  } 
  else { 
    // 200초과 300 미만에서 
    float ratio = (300.0 - distance) / 100.0; //300 보다 작은 거리이므로 300에서 distance를 빼고 100으로 나누어 ratio 값을 정함
    pwm = 255 - (int)(255 * ratio); // distance가 300에 가까워질수록 ratio가 작아지고, pwm의 값이 커짐(a.k.a 어두워짐)
  }

  analogWrite(PIN_LED, pwm);

  // 시리얼 출력
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(", distance:"); Serial.print(distance);
  Serial.print(", Max:");      Serial.print(_DIST_MAX);
  Serial.print(", PWM:");      Serial.print(pwm);
  Serial.println("");

  last_sampling_time = millis(); // 시간 동기화
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}
