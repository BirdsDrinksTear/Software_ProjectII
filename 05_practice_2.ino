#define LED_PIN 7  // GPIO 7번 핀

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // 1초 동안 LED 켜기
  // 0일 때 켜짐, 1일 때 꺼짐
  digitalWrite(LED_PIN, 0);
  delay(1000);

  // LED 5회 깜빡이기 (각 켜기 + 끄기 합쳐서 한번 깜빡일 때 0.2초)
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, 1);
    delay(100);  // 꺼짐
    digitalWrite(LED_PIN, 0);
    delay(100);  // 켜짐
    
  }

  // LED 끄고 무한 루프
  digitalWrite(LED_PIN, 1);
  while(1) {
    ; 
  }
}
