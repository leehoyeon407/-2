#define TRIG 12
#define ECHO 13
#define LED 9

unsigned long last_sampling_time = 0;
const int INTERVAL = 25;  // 25ms 샘플링 주기

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long now = millis();
  if (now - last_sampling_time >= INTERVAL) {
    last_sampling_time = now;

    // 초음파 센서로 거리 측정
    long distance = getDistance();

    // 거리 기반 LED duty 계산
    int duty = calcDuty(distance);

    // LED 제어
    analogWrite(LED, duty);

    // 시리얼 출력
    Serial.print("Distance(mm): ");
    Serial.print(distance);
    Serial.print("  Duty: ");
    Serial.println(duty);
  }
}

// 거리 측정 함수 (mm 단위)
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH); 
  long distance = duration * 0.34 / 2; // 속도: 340 m/s → 0.34 mm/us
  return distance;
}

// 거리 기반 duty 계산 함수
int calcDuty(long d) {
  // 구간별 선형 보간
  if (d <= 100) return 255; // 최소 밝기
  if (d >= 300) return 255; // 최소 밝기
  if (d == 200) return 0;   // 최대 밝기

  if (d > 100 && d < 150) {
    // 100 → 255, 150 → 127 (50%)
    return map(d, 100, 150, 255, 127);
  } else if (d >= 150 && d < 200) {
    // 150 → 127, 200 → 0
    return map(d, 150, 200, 127, 0);
  } else if (d > 200 && d < 250) {
    // 200 → 0, 250 → 127
    return map(d, 200, 250, 0, 127);
  } else if (d >= 250 && d < 300) {
    // 250 → 127, 300 → 255
    return map(d, 250, 300, 127, 255);
  }
  return 255; // default
}
