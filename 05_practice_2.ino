int ledPin = 7;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 1초 동안 켜기
  digitalWrite(ledPin, LOW);
  delay(1000);

  // 구분용 잠깐 끄기
  digitalWrite(ledPin, HIGH);
  delay(500);

  // 5번 깜빡이기
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, LOW);
    delay(200);
    digitalWrite(ledPin, HIGH);
    delay(200);
  }

  // 마지막에 LED 꺼진 상태로 멈춤
  digitalWrite(ledPin, HIGH);
  while (1) {}  // 무한루프
}
