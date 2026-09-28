#include <Arduino.h>

/* =========================
   RAIL TB6600 (comme ton test)
   ========================= */
const int STEP_PIN = 2;   // PUL-
const int DIR_PIN  = 3;   // DIR-

const long OVERSHOOT_STEPS_UP   = 1200;  // à calibrer (ex: 200 pas)
const long OVERSHOOT_STEPS_DOWN = 1200;  // à calibrer
const int  stepDelayUsOver      = 1200; // plus lent pendant l’overshoot

/* =========================
   LIMIT SWITCHES (INPUT_PULLUP)
   appuyé = LOW
   ========================= */
const int LIMIT_TOP_PIN    = 22;
const int LIMIT_BOTTOM_PIN = 23;

/* =========================
   BTS7960 - ACTUATOR
   ========================= */
const int RPWM_PIN = 44;
const int LPWM_PIN = 45;
const int REN_PIN  = 46;
const int LEN_PIN  = 47;

/* =========================
   PARAMS RAIL
   ========================= */
const int stepDelayUs = 800;   // vitesse (800–1500)
const int stepHighUs  = 20;    // >=10us TB6600

/* =========================
   PARAMS ACTUATOR
   ========================= */
const int ACTUATOR_PWM = 200;          // puissance 0..255
const unsigned long ACT_FULL_MS = 2500; // <-- AJUSTE: temps pour sortie/rentrée complète

/* =========================
   DIR constants
   ========================= */
const byte UP_DIR   = 1;
const byte DOWN_DIR = 0;

/* =========================
   LIMIT HELPERS
   ========================= */
bool topPressed()    { return digitalRead(LIMIT_TOP_PIN) == LOW; }
bool bottomPressed() { return digitalRead(LIMIT_BOTTOM_PIN) == LOW; }

/* =========================
   STEP PULSE
   ========================= */
void stepPulse(int delayUs) {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(stepHighUs);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(delayUs);
}

bool stopRequested() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == 'x' || c == 'X') {
      return true;
    }
  }

  return false;
}

bool railExtraSteps(long steps) {

  for (long i = 0; i < steps; i++) {

    if (stopRequested()) {
      return false;
    }

    stepPulse(stepDelayUsOver);
  }

  return true;
}

/* =========================
   RAIL move until limit
   ========================= */
bool railMoveToLimit(byte dir) {

  if (dir == UP_DIR && topPressed()) {
    return true;
  }

  if (dir == DOWN_DIR && bottomPressed()) {
    return true;
  }

  digitalWrite(
    DIR_PIN,
    (dir == UP_DIR) ? LOW : HIGH
  );

  while (true) {

    // STOP demandé par Serial
    if (stopRequested()) {
      return false;
    }

    if (dir == UP_DIR && topPressed()) {
      break;
    }

    if (dir == DOWN_DIR && bottomPressed()) {
      break;
    }

    stepPulse(stepDelayUs);
  }

  // Overshoot toujours interruptible par x
  if (dir == UP_DIR) {
    return railExtraSteps(OVERSHOOT_STEPS_UP);
  }
  else {
    return railExtraSteps(OVERSHOOT_STEPS_DOWN);
  }
}

/* =========================
   ACTUATOR control
   ========================= */
void actuatorStop() {
  analogWrite(RPWM_PIN, 0);
  analogWrite(LPWM_PIN, 0);
  digitalWrite(REN_PIN, LOW);
  digitalWrite(LEN_PIN, LOW);
}

void actuatorRun(bool extend, int pwm) {
  pwm = constrain(pwm, 0, 255);
  digitalWrite(REN_PIN, HIGH);
  digitalWrite(LEN_PIN, HIGH);

  if (extend) {
    analogWrite(RPWM_PIN, pwm);
    analogWrite(LPWM_PIN, 0);
  } else {
    analogWrite(RPWM_PIN, 0);
    analogWrite(LPWM_PIN, pwm);
  }
}

bool actuatorFullMove(bool extend) {

  actuatorRun(extend, ACTUATOR_PWM);

  unsigned long startTime = millis();

  while (millis() - startTime < ACT_FULL_MS) {

    if (stopRequested()) {
      actuatorStop();
      return false;
    }

    delay(10);
  }

  actuatorStop();
  return true;
}

bool liftAndRetractParallel() {

  Serial.println("GRAB: debut (rail UP + verin RETRACT)");

  // =========================
  // VERIN : commencer la rentree
  // =========================
  actuatorRun(false, ACTUATOR_PWM);

  unsigned long actuatorStart = millis();
  bool actuatorDone = false;

  // =========================
  // RAIL : commencer la montee
  // =========================
  bool railDone = topPressed();

  bool overshootStarted = false;
  long overshootCount = 0;

  // Même sens que railMoveToLimit(UP_DIR)
  digitalWrite(DIR_PIN, LOW);

  // =========================
  // BOUCLE PARALLELE
  // =========================
  while (!actuatorDone || !railDone) {

    // STOP general
    if (stopRequested()) {
      actuatorStop();

      Serial.println("GRAB: STOP (commande x)");
      return false;
    }

    // ----- VERIN -----
    if (!actuatorDone) {

      if (millis() - actuatorStart >= ACT_FULL_MS) {
        actuatorStop();
        actuatorDone = true;

        Serial.println("GRAB: verin rentre");
      }
    }

    // ----- RAIL -----
    if (!railDone) {

      if (!overshootStarted) {

        if (topPressed()) {
          overshootStarted = true;
          overshootCount = 0;
        }
        else {
          stepPulse(stepDelayUs);
        }
      }
      else {

        if (overshootCount < OVERSHOOT_STEPS_UP) {
          stepPulse(stepDelayUsOver);
          overshootCount++;
        }
        else {
          railDone = true;

          Serial.println("GRAB: rail en haut");
        }
      }
    }
  }

  actuatorStop();

  Serial.println("GRAB: termine");
  return true;
}

bool homeArm() {

  Serial.println("HOME: debut (mouvements paralleles)");

  /* =========================
     DEMARRAGE VERIN
     ========================= */
  actuatorRun(false, ACTUATOR_PWM);   // rentrer

  unsigned long actuatorStart = millis();
  bool actuatorDone = false;

  /* =========================
     DEMARRAGE RAIL
     ========================= */
  bool railDone = bottomPressed();

  bool overshootStarted = false;
  long overshootCount = 0;

  // Même sens que railMoveToLimit(DOWN_DIR)
  digitalWrite(DIR_PIN, HIGH);

  /* =========================
     BOUCLE PARALLELE
     ========================= */
  while (!actuatorDone || !railDone) {

    // STOP général
    if (stopRequested()) {
      actuatorStop();

      Serial.println("HOME: STOP (commande x)");
      return false;
    }

    /* ----- VERIN ----- */
    if (!actuatorDone) {

      if (millis() - actuatorStart >= ACT_FULL_MS) {
        actuatorStop();
        actuatorDone = true;

        Serial.println("HOME: verin rentre");
      }
    }

    /* ----- RAIL ----- */
    if (!railDone) {

      // Descente normale jusqu'au fin de course
      if (!overshootStarted) {

        if (bottomPressed()) {
          overshootStarted = true;
          overshootCount = 0;
        }
        else {
          stepPulse(stepDelayUs);
        }
      }

      // Overshoot après fin de course
      else {

        if (overshootCount < OVERSHOOT_STEPS_DOWN) {
          stepPulse(stepDelayUsOver);
          overshootCount++;
        }
        else {
          railDone = true;

          Serial.println("HOME: rail en bas");
        }
      }
    }
  }

  actuatorStop();

  Serial.println("HOME: termine");
  return true;
}


/* =========================
   UI
   ========================= */
void printMenu() {
  Serial.println("\n=== SIMPLE CONTROL ===");
  Serial.println("w : Rail MONTER jusqu'au fin de course HAUT");
  Serial.println("s : Rail DESCENDRE jusqu'au fin de course BAS");
  Serial.println("e : Verin SORTIR complet (temps)");
  Serial.println("r : Verin RENTRER complet (temps)");
  Serial.println("h : HOME (verin rentre + rail en bas)");
  Serial.println("g : GRAB (rail monte + verin rentre en parallele)");
  Serial.println("======================\n");
}

void setup() {
  Serial.begin(115200);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  pinMode(LIMIT_TOP_PIN, INPUT_PULLUP);
  pinMode(LIMIT_BOTTOM_PIN, INPUT_PULLUP);

  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(LEN_PIN, OUTPUT);
  actuatorStop();

  printMenu();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();

  if (c == 'w') {

    Serial.println("Rail: MONTER...");

    bool completed = railMoveToLimit(UP_DIR);

    if (completed) {
      Serial.println("Rail: STOP (fin de course haut)");
    }
    else {
      Serial.println("Rail: STOP (commande x)");
    }
  }

  else if (c == 's') {

    Serial.println("Rail: DESCENDRE...");

    bool completed = railMoveToLimit(DOWN_DIR);

    if (completed) {
      Serial.println("Rail: STOP (fin de course bas)");
    }
    else {
      Serial.println("Rail: STOP (commande x)");
    }
  }
  else if (c == 'e') {

    Serial.println("Verin: SORTIR complet...");

    bool completed = actuatorFullMove(true);

    if (completed) {
      Serial.println("Verin: STOP (fin mouvement)");
    }
    else {
      Serial.println("Verin: STOP (commande x)");
    }
  }

  else if (c == 'r') {

    Serial.println("Verin: RENTRER complet...");

    bool completed = actuatorFullMove(false);

    if (completed) {
      Serial.println("Verin: STOP (fin mouvement)");
    }
    else {
      Serial.println("Verin: STOP (commande x)");
    }
  }

  else if (c == 'h' || c == 'H') {
    homeArm();
  }
  else if (c == 'g' || c == 'G') {
    liftAndRetractParallel();
  }
  else if (c == 'x' || c == 'X') {
    actuatorStop();
    Serial.println("STOP: aucun mouvement actif");
  }
  else if (c == '\n' || c == '\r') {
    // ignore
  }
  else {
    printMenu();
  }
}
