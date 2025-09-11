#define PIN_LED 13   // LED 핀 정의

unsigned int count = 0;   
bool toggle = false;      

void setup() {
  Serial.begin(9600);     
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  count++;                      
  Serial.print("Count: ");         
  Serial.println(count);

  toggle = !toggle;                
  digitalWrite(PIN_LED, toggle);
  delay(1000);                     
