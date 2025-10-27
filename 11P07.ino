#include <Servo.h>

// 핀 정의 (과제 요구사항에 따른 핀 번호)
#define PIN_LED      9    // 9번 핀: 거리 측정 성공/실패 검증용 LED
#define PIN_TRIG     12   // 12번 핀: 초음파 센서 트리거
#define PIN_ECHO     13   // 13번 핀: 초음파 센서 에코
#define PIN_SERVO    10   // 10번 핀: 서보 모터 신호

// 상수 정의
#define SND_VEL        346.0      // 음속 (mm/ms, 23도 기준)
#define INTERVAL       25         // 샘플링 주기 (ms)
#define PULSE_DURATION 10         // TRIG 펄스 폭 (us)
#define TIMEOUT        ((INTERVAL / 2) * 1000.0) // pulseIn 타임아웃 (us)
#define SCALE          (0.001 * 0.5 * SND_VEL)  // 거리 변환 상수 (us * SCALE -> mm)

// 거리/필터 관련 상수 (단위: mm)
#define _DIST_MIN      180.0      // 최소 측정 거리 (18cm)
#define _DIST_MAX      360.0      // 최대 측정 거리 (36cm)
#define _EMA_ALPHA     0.35       // EMA 필터 계수 (조절 가능: 0.0 ~ 1.0)

// 서보 모터 관련 상수 (Pulse Width Modulation, 마이크로초)
#define _DUTY_MIN      500        // 서보 0도 PWM 값 (대부분 500~1000, 500으로 설정)
#define _DUTY_NEU      1500       // 서보 중립 90도 PWM 값
#define _DUTY_MAX      2500       // 서보 180도 PWM 값 (대부분 2000~2500, 2500으로 설정)

// 전역 변수
float dist_prev = _DIST_MIN;         // 이전 유효 거리 값
float dist_ema  = _DIST_MIN;         // EMA 필터링된 거리
unsigned long last_sampling_time = 0; // 마지막 샘플링 시간 기록

Servo myservo; // 서보 객체 생성

/**
 * 초음파 센서를 사용하여 거리를 측정하는 함수 (단위: mm)
 */
float USS_measure(int TRIG, int ECHO) {
  // 1. 트리거 펄스 생성
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  // 2. 에코 펄스 길이 측정 및 거리로 변환
  // pulseIn()은 TIMEOUT(25ms/2 = 12.5ms = 12500us) 안에 신호가 없으면 0을 반환.
  // 0 반환 시, 거리도 0.0mm가 됨.
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // mm
}

/**
 * 측정된 거리(d)를 서보 각도(0~180도)로 매핑하는 함수 (과제 요구사항)
 */
float mapDistanceToAngle(float d) {
  // 거리 18cm (180mm) 이하: 0도
  if (d <= _DIST_MIN) return 0.0;
  // 거리 36cm (360mm) 이상: 180도
  if (d >= _DIST_MAX) return 180.0;
  
  // 거리 18cm ~ 36cm 사이: 거리에 비례하여 0도 ~ 180도 연속 변환
  return (d - _DIST_MIN) * (180.0 / (_DIST_MAX - _DIST_MIN));
}

void setup() {
  // 핀 모드 설정
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW); // Trig 핀 초기 상태 LOW

  // 서보 초기화 및 중립 위치 (90도) 설정
  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX); // 최소/최대 PWM 값 설정 추가
  myservo.writeMicroseconds(_DUTY_NEU);

  // LED 초기 상태: 꺼짐 (Active-LOW 가정 시 HIGH)
  digitalWrite(PIN_LED, HIGH);
  
  // 시리얼 통신 시작 (플로터 및 디버깅용)
  Serial.begin(57600);
}

void loop() {
  // INTERVAL(25ms)마다 실행되도록 시간 제어
  if (millis() < last_sampling_time + INTERVAL) return;

  // 1. 거리 측정
  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 2. **범위 필터** 적용 (측정 실패 또는 범위 초과/미달 시 이전 값 사용)
  float dist_filtered;
  bool is_valid_measurement = (dist_raw > 0.0 && dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX);
  
  if (!is_valid_measurement) {
    dist_filtered = dist_prev; // 이전 유효한 값 사용
  } else {
    dist_filtered = dist_raw;  // 유효한 값 사용
    dist_prev = dist_raw;      // 이전 값 업데이트
  }

  // 3. **EMA 필터** 적용
  dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

  // 4. 거리 -> 각도 변환 및 서보 제어
  float angle = mapDistanceToAngle(dist_ema);
  
  // 서보 PWM 값 계산 (0도:_DUTY_MIN ~ 180도:_DUTY_MAX)
  int duty = (int)(_DUTY_MIN + (angle / 180.0) * (_DUTY_MAX - _DUTY_MIN));
  
  myservo.writeMicroseconds(duty);

  // 5. PIN_LED (9번 핀) 검증 (거리 측정 범위 안에 들어왔을 때 LED 켜기)
  // Active-LOW 연결을 가정하여 LOW일 때 켜짐
  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX)
    digitalWrite(PIN_LED, LOW);  // 켜짐 (거리 OK)
  else
    digitalWrite(PIN_LED, HIGH); // 꺼짐 (거리 범위 밖/측정 실패)

  // 6. 시리얼 출력 (플로터 고려)
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",dist:"); Serial.print(dist_raw);
  Serial.print(",ema:");  Serial.print(dist_ema);
  Serial.print(",Servo:");Serial.print(myservo.read()); // read()는 각도(0~180)를 반환
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  last_sampling_time += INTERVAL;
}
#include <Servo.h>

// 핀 정의 (과제 요구사항에 따른 핀 번호)
#define PIN_LED      9    // 9번 핀: 거리 측정 성공/실패 검증용 LED
#define PIN_TRIG     12   // 12번 핀: 초음파 센서 트리거
#define PIN_ECHO     13   // 13번 핀: 초음파 센서 에코
#define PIN_SERVO    10   // 10번 핀: 서보 모터 신호

// 상수 정의
#define SND_VEL        346.0      // 음속 (mm/ms, 23도 기준)
#define INTERVAL       25         // 샘플링 주기 (ms)
#define PULSE_DURATION 10         // TRIG 펄스 폭 (us)
#define TIMEOUT        ((INTERVAL / 2) * 1000.0) // pulseIn 타임아웃 (us)
#define SCALE          (0.001 * 0.5 * SND_VEL)  // 거리 변환 상수 (us * SCALE -> mm)

// 거리/필터 관련 상수 (단위: mm)
#define _DIST_MIN      180.0      // 최소 측정 거리 (18cm)
#define _DIST_MAX      360.0      // 최대 측정 거리 (36cm)
#define _EMA_ALPHA     0.35       // EMA 필터 계수 (조절 가능: 0.0 ~ 1.0)

// 서보 모터 관련 상수 (Pulse Width Modulation, 마이크로초)
#define _DUTY_MIN      500        // 서보 0도 PWM 값 (대부분 500~1000, 500으로 설정)
#define _DUTY_NEU      1500       // 서보 중립 90도 PWM 값
#define _DUTY_MAX      2500       // 서보 180도 PWM 값 (대부분 2000~2500, 2500으로 설정)

// 전역 변수
float dist_prev = _DIST_MIN;         // 이전 유효 거리 값
float dist_ema  = _DIST_MIN;         // EMA 필터링된 거리
unsigned long last_sampling_time = 0; // 마지막 샘플링 시간 기록

Servo myservo; // 서보 객체 생성

/**
 * 초음파 센서를 사용하여 거리를 측정하는 함수 (단위: mm)
 */
float USS_measure(int TRIG, int ECHO) {
  // 1. 트리거 펄스 생성
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  // 2. 에코 펄스 길이 측정 및 거리로 변환
  // pulseIn()은 TIMEOUT(25ms/2 = 12.5ms = 12500us) 안에 신호가 없으면 0을 반환.
  // 0 반환 시, 거리도 0.0mm가 됨.
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // mm
}

/**
 * 측정된 거리(d)를 서보 각도(0~180도)로 매핑하는 함수 (과제 요구사항)
 */
float mapDistanceToAngle(float d) {
  // 거리 18cm (180mm) 이하: 0도
  if (d <= _DIST_MIN) return 0.0;
  // 거리 36cm (360mm) 이상: 180도
  if (d >= _DIST_MAX) return 180.0;
  
  // 거리 18cm ~ 36cm 사이: 거리에 비례하여 0도 ~ 180도 연속 변환
  return (d - _DIST_MIN) * (180.0 / (_DIST_MAX - _DIST_MIN));
}

void setup() {
  // 핀 모드 설정
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW); // Trig 핀 초기 상태 LOW

  // 서보 초기화 및 중립 위치 (90도) 설정
  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX); // 최소/최대 PWM 값 설정 추가
  myservo.writeMicroseconds(_DUTY_NEU);

  // LED 초기 상태: 꺼짐 (Active-LOW 가정 시 HIGH)
  digitalWrite(PIN_LED, HIGH);
  
  // 시리얼 통신 시작 (플로터 및 디버깅용)
  Serial.begin(57600);
}

void loop() {
  // INTERVAL(25ms)마다 실행되도록 시간 제어
  if (millis() < last_sampling_time + INTERVAL) return;

  // 1. 거리 측정
  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 2. **범위 필터** 적용 (측정 실패 또는 범위 초과/미달 시 이전 값 사용)
  float dist_filtered;
  bool is_valid_measurement = (dist_raw > 0.0 && dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX);
  
  if (!is_valid_measurement) {
    dist_filtered = dist_prev; // 이전 유효한 값 사용
  } else {
    dist_filtered = dist_raw;  // 유효한 값 사용
    dist_prev = dist_raw;      // 이전 값 업데이트
  }

  // 3. **EMA 필터** 적용
  dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

  // 4. 거리 -> 각도 변환 및 서보 제어
  float angle = mapDistanceToAngle(dist_ema);
  
  // 서보 PWM 값 계산 (0도:_DUTY_MIN ~ 180도:_DUTY_MAX)
  int duty = (int)(_DUTY_MIN + (angle / 180.0) * (_DUTY_MAX - _DUTY_MIN));
  
  myservo.writeMicroseconds(duty);

  // 5. PIN_LED (9번 핀) 검증 (거리 측정 범위 안에 들어왔을 때 LED 켜기)
  // Active-LOW 연결을 가정하여 LOW일 때 켜짐
  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX)
    digitalWrite(PIN_LED, LOW);  // 켜짐 (거리 OK)
  else
    digitalWrite(PIN_LED, HIGH); // 꺼짐 (거리 범위 밖/측정 실패)

  // 6. 시리얼 출력 (플로터 고려)
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",dist:"); Serial.print(dist_raw);
  Serial.print(",ema:");  Serial.print(dist_ema);
  Serial.print(",Servo:");Serial.print(myservo.read()); // read()는 각도(0~180)를 반환
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  last_sampling_time += INTERVAL;
}
