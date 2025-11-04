// tune
#define CELL_CM        20
#define WALL_THRESH_CM 18
#define TURN_TIME_MS   350
#define ADJUST_MS      120
#define FWD_SPEED      160
#define TURN_SPEED     150
#define READS          3   

// ultrasonic pin
const int trigFront = A0;
const int echoFront = A1;

const int trigLeft  = A2;
const int echoLeft  = A3;

const int trigRight = A4;
const int echoRight = A5;

// motor drivver pin
const int in1R = 7;   // Right IN1
const int in2R = 8;   // Right IN2
const int enR  = 6;   // Right ENA (PWM)

const int in1L = 4;   // Left IN3
const int in2L = 12;  // Left IN4
const int enL  = 5;   // Left ENB (PWM)

// ultrasonic read
long readUS(const int trig, const int echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  unsigned long duration = pulseIn(echo, HIGH, 50000UL);
  if (duration == 0) return 400;                  
  long cm = (long)(duration * 0.034 / 2.0);
  if (cm < 2) cm = 2; if (cm > 400) cm = 400;     
  return cm;
}

long readMedianCM(const int trig, const int echo) { 
  long a[READS];
  for (int i = 0; i < READS; i++) { 
    a[i] = readUS(trig, echo); 
    delay(10); 
  }
  for (int i = 0; i < READS-1; i++)
    for (int j = i+1; j < READS; j++)
      if (a[j] < a[i]) { long t=a[i]; a[i]=a[j]; a[j]=t; }
  return a[READS/2];
}

// motor drive
void setMotor(int in1, int in2, int en, int pwm) {
  if (pwm > 255) pwm = 255;
  if (pwm < -255) pwm = -255;

  if (pwm > 0) {                  
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(en, pwm);
  } else if (pwm < 0) {           
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(en, -pwm);
  } else {                        
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0);
  }
}

void stopNow() {
  setMotor(in1L, in2L, enL, 0);
  setMotor(in1R, in2R, enR, 0);
}

void forward(int pwm) {
  setMotor(in1L, in2L, enL, pwm);
  setMotor(in1R, in2R, enR, pwm);
}

void turnLeft(int pwm) {
  setMotor(in1L, in2L, enL, -pwm);
  setMotor(in1R, in2R, enR,  pwm);
}

void turnRight(int pwm) {
  setMotor(in1L, in2L, enL,  pwm);
  setMotor(in1R, in2R, enR, -pwm);
}

// set up
void setup() {
  // Ultrasonic
  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft,  OUTPUT); pinMode(echoLeft,  INPUT);
  pinMode(trigRight, OUTPUT); pinMode(echoRight, INPUT);
  digitalWrite(trigFront, LOW);
  digitalWrite(trigLeft,  LOW);
  digitalWrite(trigRight, LOW);

  // Motors
  pinMode(in1R, OUTPUT); pinMode(in2R, OUTPUT); pinMode(enR, OUTPUT);
  pinMode(in1L, OUTPUT); pinMode(in2L, OUTPUT); pinMode(enL, OUTPUT);

  stopNow();
  delay(500);
}

//loop
// Left-hand rule: ซ้ายว่าง => เลี้ยวซ้าย, หน้าตัน => เลี้ยวขวา, ไม่งั้นไปตรง
void loop() {
  long dF = readMedianCM(trigFront, echoFront);
  long dL = readMedianCM(trigLeft,  echoLeft);
  long dR = readMedianCM(trigRight, echoRight); // เผื่อใช้ภายหลัง

  bool wallF = (dF < WALL_THRESH_CM);
  bool wallL = (dL < WALL_THRESH_CM);

  if (!wallL) { // ซ้ายโล่ง => เลี้ยวซ้าย
    stopNow();              delay(ADJUST_MS);
    turnLeft(TURN_SPEED);   delay(TURN_TIME_MS);
    stopNow();              delay(ADJUST_MS);
  } 
  else if (wallF) { // หน้าตัน => เลี้ยวขวา
    stopNow();              delay(ADJUST_MS);
    turnRight(TURN_SPEED);  delay(TURN_TIME_MS);
    stopNow();              delay(ADJUST_MS);
  } 
  else { // ไปตรง
    forward(FWD_SPEED);
    delay(120);
  }
}