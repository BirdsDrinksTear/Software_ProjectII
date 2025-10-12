#define PIN_LED   9
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0          
#define INTERVAL 25            
#define PULSE_DURATION 10      
#define _DIST_MIN 100         
#define _DIST_MAX 300         

#define TIMEOUT ((INTERVAL / 2) * 1000.0)   
#define SCALE (0.001 * 0.5 * SND_VEL)       
#define _EMA_ALPHA 0.5                      // EMA 
#define MEDIAN_N 30                         // 중위수 필터 (3, 10, 30 택1하기)

unsigned long last_sampling_time = 0;
float dist_ema = _DIST_MAX;

// 중위수 필터용 링버퍼
float dist_samples[MEDIAN_N];
int   sample_index = 0;
int   sample_count = 0; // 현재 채워진 개수

// 함수 선언
float USS_measure(int TRIG, int ECHO);
float medianFilter(const float arr[], int n);

// 초음파 측정
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  unsigned long dur = pulseIn(ECHO, HIGH, (unsigned long)TIMEOUT);
  return (float)dur * SCALE; // us * 계수 = mm
}

// 최근 n개로 중위수 계산하기기
float medianFilter(const float arr[], int n) {
  float temp_arr[MEDIAN_N];
  for (int i = 0; i < n; i++) temp_arr[i] = arr[i];

  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (temp_arr[j] > temp_arr[j + 1]) {
        float tmp = temp_arr[j];
        temp_arr[j] = temp_arr[j + 1];
        temp_arr[j + 1] = tmp;
      }
    }
  }
  if (n % 2) return temp_arr[n / 2];
  return 0.5f * (temp_arr[n / 2 - 1] + temp_arr[n / 2]);
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  Serial.begin(57600);

  // 안정화를 위해 버퍼 초기화
  for (int i = 0; i < MEDIAN_N; i++) dist_samples[i] = _DIST_MAX;
}

void loop() {
  // 고정 주기 샘플링
  if (millis() - last_sampling_time < INTERVAL) return;
  last_sampling_time += INTERVAL;

  // 원본 측정값
  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 중위수 필터로 median 계산
  dist_samples[sample_index] = dist_raw;
  sample_index = (sample_index + 1) % MEDIAN_N;
  if (sample_count < MEDIAN_N) sample_count++;
  float dist_median = medianFilter(dist_samples, sample_count);

  // 비교용 EMA 
  dist_ema = _EMA_ALPHA * dist_raw + (1.0f - _EMA_ALPHA) * dist_ema;

  // 출력방식
  Serial.print("Min:");     Serial.print(_DIST_MIN);
  Serial.print(",raw:");    Serial.print(dist_raw);
  Serial.print(",ema:");    Serial.print(dist_ema);
  Serial.print(",median:"); Serial.print(dist_median);
  Serial.print(",Max:");    Serial.print(_DIST_MAX);
  Serial.println();

  // LED
  if (dist_median < _DIST_MIN || dist_median > _DIST_MAX)
    digitalWrite(PIN_LED, HIGH);
  else
    digitalWrite(PIN_LED, LOW);
}
