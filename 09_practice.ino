// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// Parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

#define N_MEDIAN 30  // 최근 샘플 수 (3, 10, 30 등으로 실험 가능)

// Global variables
unsigned long last_sampling_time;
float dist_raw;
float dist_median;

float buffer[N_MEDIAN]; // 중위수 필터용 버퍼
int buffer_index = 0;
bool buffer_filled = false;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);
}

void loop() {
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // 1. 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 2. 버퍼에 새로운 값 추가
  buffer[buffer_index] = dist_raw;
  buffer_index = (buffer_index + 1) % N_MEDIAN;

  // 버퍼가 다 찼는지 확인
  if (buffer_index == 0) buffer_filled = true;

  // 3. 중위수 계산
  int count = buffer_filled ? N_MEDIAN : buffer_index;
  dist_median = computeMedian(buffer, count);

  // 4. 시리얼 플로터 출력
  Serial.print("dist_raw:");    Serial.print(dist_raw);
  Serial.print("\t");
  Serial.print("dist_median:"); Serial.println(dist_median);

  // 5. LED 동작 예시 (중위수 기준 100mm 이하일 때 ON)
  if (dist_median < 100)
    digitalWrite(PIN_LED, LOW);
  else
    digitalWrite(PIN_LED, HIGH);

  last_sampling_time += INTERVAL;
}

// 초음파 거리 측정 함수
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}

// 중위수 계산 함수
float computeMedian(float arr[], int len) {
  float temp[len];
  memcpy(temp, arr, len * sizeof(float));
  for (int i = 0; i < len - 1; i++) {
    for (int j = i + 1; j < len; j++) {
      if (temp[i] > temp[j]) {
        float t = temp[i];
        temp[i] = temp[j];
        temp[j] = t;
      }
    }
  }
  if (len % 2 == 0)
    return (temp[len / 2 - 1] + temp[len / 2]) / 2.0;
  else
    return temp[len / 2];
}// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// Parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

#define N_MEDIAN 30  // 최근 샘플 수 (3, 10, 30 등으로 실험 가능)

// Global variables
unsigned long last_sampling_time;
float dist_raw;
float dist_median;

float buffer[N_MEDIAN]; // 중위수 필터용 버퍼
int buffer_index = 0;
bool buffer_filled = false;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);
}

void loop() {
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // 1. 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 2. 버퍼에 새로운 값 추가
  buffer[buffer_index] = dist_raw;
  buffer_index = (buffer_index + 1) % N_MEDIAN;

  // 버퍼가 다 찼는지 확인
  if (buffer_index == 0) buffer_filled = true;

  // 3. 중위수 계산
  int count = buffer_filled ? N_MEDIAN : buffer_index;
  dist_median = computeMedian(buffer, count);

  // 4. 시리얼 플로터 출력
  Serial.print("dist_raw:");    Serial.print(dist_raw);
  Serial.print("\t");
  Serial.print("dist_median:"); Serial.println(dist_median);

  // 5. LED 동작 예시 (중위수 기준 100mm 이하일 때 ON)
  if (dist_median < 100)
    digitalWrite(PIN_LED, LOW);
  else
    digitalWrite(PIN_LED, HIGH);

  last_sampling_time += INTERVAL;
}

// 초음파 거리 측정 함수
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}

// 중위수 계산 함수
float computeMedian(float arr[], int len) {
  float temp[len];
  memcpy(temp, arr, len * sizeof(float));
  for (int i = 0; i < len - 1; i++) {
    for (int j = i + 1; j < len; j++) {
      if (temp[i] > temp[j]) {
        float t = temp[i];
        temp[i] = temp[j];
        temp[j] = t;
      }
    }
  }
  if (len % 2 == 0)
    return (temp[len / 2 - 1] + temp[len / 2]) / 2.0;
  else
    return temp[len / 2];
}
