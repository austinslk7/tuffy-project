// ค่อย tune
#define CELL_CM        20
#define WALL_THRESH_CM 18
#define TURN_TIME_MS   350
#define ADJUST_MS      80
#define FWD_SPEED      160
#define TURN_SPEED     150
#define READS          3

// ultasonic pin
const int trigFront = 6;
const int echoFront = 2;

const int trigLeft  = 12;
const int echoLeft  = 13;

const int trigRight = 10;
const int echoRight = 11;

// motor drivver pin
const int in1R = 7;
const int in2R = 8;
const int enR  = 9;

const int in1L = 4;
const int in2L = 5;
const int enL  = 3;

// ultrasonic read
long readUS(const int trig, const int echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 25000UL);
  if (duration == 0) return 400;
  return (long)(duration * 0.034 / 2.0);
}

long readMedianCM(const int trig, const int echo) {   // find read median
  long a[READS];
  for (int i = 0; i < READS; i++) { a[i] = readUS(trig, echo); delay(10); }
  for (int i = 0; i < READS-1; i++)
    for (int j = i+1; j < READS; j++)
      if (a[j] < a[i]) { long t=a[i]; a[i]=a[j]; a[j]=t; }
  return a[READS/2];
}

// motor drive
void setMotor(int in1, int in2, int en, int pwm) {
  if (pwm > 0) { digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(en, pwm); }
  else if (pwm < 0) { digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(en, -pwm); }
  else { digitalWrite(in1, LOW);  digitalWrite(in2, LOW); analogWrite(en, 0); }
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

// setup
void setup() {
  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft,  OUTPUT); pinMode(echoLeft,  INPUT);
  pinMode(trigRight, OUTPUT); pinMode(echoRight, INPUT);

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
  long dR = readMedianCM(trigRight, echoRight); // เก็บไว้

  bool wallF = (dF < WALL_THRESH_CM);
  bool wallL = (dL < WALL_THRESH_CM);

  if (!wallL) {
    stopNow();            delay(ADJUST_MS);
    turnLeft(TURN_SPEED); delay(TURN_TIME_MS);
    stopNow();            delay(ADJUST_MS);
  } else if (wallF) {
    stopNow();             delay(ADJUST_MS);
    turnRight(TURN_SPEED); delay(TURN_TIME_MS);
    stopNow();             delay(ADJUST_MS);
  } else {
    forward(FWD_SPEED);
    delay(120);
  }
}