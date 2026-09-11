// ===== IBT-2 (МОТОРЫ) =====
const int IBT_L_LPWM = 31; 
const int IBT_L_RPWM = 30; 
const int IBT_R_LPWM = 29; 
const int IBT_R_RPWM = 25; 

// ===== СОНАРЫ =====
const int TRIG_FRONT = 3; const int ECHO_FRONT = 7; 
const int TRIG_BACK  = 20; const int ECHO_BACK  = 21; 
const int TRIG_RIGHT = 8; const int ECHO_RIGHT = 13; 
const int TRIG_LEFT  = 12; const int ECHO_LEFT  = 17; 

// ===== КНОПКИ И БУЗЗЕР =====
const int BTN_RIGHT = 15; // Кнопка Вправо / Прямо (при двойном клике)
const int BTN_LEFT  = 10;  // Кнопка Влево
const int BTN_180   = 14; // Кнопка Разворот 180
const int BTN_FRONT = 9;  // Новая кнопка: Атака спереди
const int BUZZER    = 19;  // Пин твоего буззера

// ===== НАСТРОЙКИ ПОВЕДЕНИЯ =====
const int ATTACK_DIST = 26;   
const int SPEED = 300;        

// НАСТРОЙКИ ПЛАВНОГО ПУСКА
const int START_SPEED = 255;  
const int ACCEL_STEP = 80;    
int currentForwardSpeed = 0;  

bool frontAttackMode = false;       
unsigned long frontLossTimer = 0;   
const int FRONT_LOSS_TIMEOUT = 400; 

// Твои откалиброванные значения времени поворотов
const int TURN_90_TIME  = 148;  
const int TURN_180_TIME = 260; 

// НАСТРОЙКИ ПОИСКА (МАЛЕНЬКИЕ ШАЖОЧКИ)
const int SEARCH_SPEED = 230;   
const int SWEEP_INTERVAL = 140; 
unsigned long sweepTimer = 0;   
bool sweepRight = true;         

// Переменные для тактики стартов
// 0 - ждем кнопку, 1 - вправо, 2 - влево, 3 - 180, 4 - прямо (двойной клик), 5 - прямо (кнопка 9)
int startStrategy = 0; 
bool robotActive = false;

// ------------ МОТОРЫ ----------------
void stopMotors() {
  analogWrite(IBT_L_LPWM, 0);  analogWrite(IBT_L_RPWM, 0);
  analogWrite(IBT_R_LPWM, 0);  analogWrite(IBT_R_RPWM, 0);
}

void moveForward(int spd) {
  analogWrite(IBT_L_LPWM, 0);    analogWrite(IBT_L_RPWM, spd);
  analogWrite(IBT_R_LPWM, spd);  analogWrite(IBT_R_RPWM, 0);
}

void turnLeft(int spd) {
    analogWrite(IBT_L_LPWM, 0);    analogWrite(IBT_L_RPWM, spd);
  analogWrite(IBT_R_LPWM, 0);    analogWrite(IBT_R_RPWM, spd);
}

void turnRight(int spd) {
    analogWrite(IBT_L_LPWM, spd);  analogWrite(IBT_L_RPWM, 0);
  analogWrite(IBT_R_LPWM, spd);  analogWrite(IBT_R_RPWM, 0);

}

// ------------ СОНАР ----------------
long readSonar(int trig, int echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

// ------------ ГРОМКИЙ СИГНАЛ БУЗЗЕРА ----------------
void beep(int duration) {
  tone(BUZZER, 4000); // Громкая частота 4 кГц
  delay(duration);
  noTone(BUZZER);     
}

// ------------ СТАРТОВЫЙ ОТСЧЕТ И РЫВОК ВПЕРЕД ----------------
void runStartCountdown() {
  // 5 секунд обязательного отсчета
  for (int i = 0; i < 5; i++) {
    beep(100);
    delay(820); 
  }
  
  // Финальный писк перед стартом
  beep(300);
  
  // 1. Выполнение тактического ПОВОРОТА
  if (startStrategy == 1) {        // Поворот вправо 90
    turnRight(SPEED);  delay(TURN_90_TIME);
  } else if (startStrategy == 2) { // Поворот влево 90
    turnLeft(SPEED);   delay(TURN_90_TIME);
  } else if (startStrategy == 3) { // Поворот на 180
    turnLeft(SPEED);   delay(TURN_180_TIME);
  } else if (startStrategy == 4 || startStrategy == 5) { 
    // Двойной клик или кнопка "Атака спереди" — уже смотрим прямо, пропускаем фазу поворота
  }
  
  // 2. АГРЕССИВНЫЙ РЫВОК ВПЕРЕД (на полной скорости 290 мс)
  moveForward(SPEED); 
  delay(190); 
  
  stopMotors();       // Короткий сброс инерции перед активацией датчиков
  robotActive = true; // Переходим в полноценный боевой режим!
}

// ------------ SETUP ----------------
void setup() {
  Serial.begin(9600);

  pinMode(IBT_L_LPWM, OUTPUT); pinMode(IBT_L_RPWM, OUTPUT);
  pinMode(IBT_R_LPWM, OUTPUT); pinMode(IBT_R_RPWM, OUTPUT);

  pinMode(TRIG_FRONT, OUTPUT); pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_BACK, OUTPUT);  pinMode(ECHO_BACK, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT); pinMode(ECHO_RIGHT, INPUT);
  pinMode(TRIG_LEFT, OUTPUT);  pinMode(ECHO_LEFT, INPUT);

  // Настройка кнопок на замыкание к GND
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_180, INPUT_PULLUP);
  pinMode(BTN_FRONT, INPUT_PULLUP); // Настройка новой кнопки
  pinMode(BUZZER, OUTPUT);

  stopMotors();
}

// ------------ LOOP ----------------
void loop() {
  
  // === БЛОК ОЖИДАНИЯ НАЖАТИЯ КНОПКИ СТАРТА ===
  if (!robotActive) {
    // Кнопка Вправо / Прямо (пин 15)
    if (digitalRead(BTN_RIGHT) == LOW) {
      beep(100);
      unsigned long clickTimer = millis();
      bool doubleClick = false;
      delay(200); // Защита от дребезга контактов
      
      // Ловим второй клик в течение 400 мс
      while (millis() - clickTimer < 400) {
        if (digitalRead(BTN_RIGHT) == LOW) {
          doubleClick = true;
          beep(100);
          break;
        }
      }
      
      if (doubleClick) {
        startStrategy = 4; // Тактика: Ехать прямо
      } else {
        startStrategy = 1; // Тактика: Поворот вправо
      }
      runStartCountdown();
    }
    
    // Кнопка Влево (пин 10)
    else if (digitalRead(BTN_LEFT) == LOW) {
      beep(100);
      startStrategy = 2; // Тактика: Поворот влево
      delay(200);
      runStartCountdown();
    }
    
    // Кнопка 180 (пин 14)
    else if (digitalRead(BTN_180) == LOW) {
      beep(100);
      startStrategy = 3; // Тактика: Разворот назад
      delay(200);
      runStartCountdown();
    }

    // Новая кнопка: Атака спереди (пин 9)
    else if (digitalRead(BTN_FRONT) == LOW) {
      beep(100);
      startStrategy = 5; // Тактика: Прямая быстрая атака спереди
      delay(200);
      runStartCountdown();
    }
    
    return; // Блокируем запуск основного алгоритма боя до выбора тактики
  }


  // =========================================================
  // БЛОК 1: ДИНАМИЧЕСКИЙ ОПРОС ДАТЧИКОВ И РЕАКЦИЯ НА ЦЕЛЬ
  // =========================================================
  long dFront = readSonar(TRIG_FRONT, ECHO_FRONT);
  long dBack  = readSonar(TRIG_BACK,  ECHO_BACK);
  long dRight = readSonar(TRIG_RIGHT, ECHO_RIGHT);
  long dLeft  = readSonar(TRIG_LEFT,  ECHO_LEFT);

  // ✅ СПЕРЕДИ — АТАКА С УСКОРЕНИЕМ
  if (dFront > 0 && dFront < ATTACK_DIST) {
    frontAttackMode = true;
    frontLossTimer = millis(); 

    if (currentForwardSpeed == 0) {
      currentForwardSpeed = START_SPEED;
    } else if (currentForwardSpeed < SPEED) {
      currentForwardSpeed += ACCEL_STEP;
      if (currentForwardSpeed > SPEED) currentForwardSpeed = SPEED; 
    }
    moveForward(currentForwardSpeed);
  }

  // ↩️ СЛЕВА — ПОВОРОТ НАЦЕЛИВАНИЯ
  else if (dLeft > 0 && dLeft < ATTACK_DIST) {
    frontAttackMode = false;
    currentForwardSpeed = 0; 
    turnLeft(SPEED);
    delay(TURN_90_TIME);
    stopMotors();
  }

  // ↪️ СПРАВА — ПОВОРОТ НАЦЕЛИВАНИЯ
  else if (dRight > 0 && dRight < ATTACK_DIST) {
    frontAttackMode = false;
    currentForwardSpeed = 0; 
    turnRight(SPEED);
    delay(TURN_90_TIME);
    stopMotors();
  }

  // 🔄 СЗАДИ — БЫСТРЫЙ РАЗВОРOT НА 180°
  else if (dBack > 0 && dBack < ATTACK_DIST) {
    frontAttackMode = false;
    currentForwardSpeed = 0; 
    turnLeft(SPEED); 
    delay(TURN_180_TIME);
    stopMotors(); 
  }

  // =========================================================
  // БЛОК 2: ОТРАБОТКА ПАМЯТИ (ДОЖИМАЕМ ПОТЕРЯННУЮ ЦЕЛЬ)
  // =========================================================
  else if (frontAttackMode) {
    if (millis() - frontLossTimer < FRONT_LOSS_TIMEOUT) {
      moveForward(currentForwardSpeed);
    } else {
      frontAttackMode = false; 
      currentForwardSpeed = 0;
    }
  }

  // =========================================================
  // БЛОК 3: НИКОГО НЕ ВИЖУ — ПОИСК ИМПУЛЬСНЫМИ ШАЖОЧКАМИ
  // =========================================================
  else {
    currentForwardSpeed = 0;
    
    if (millis() - sweepTimer > SWEEP_INTERVAL) {
      sweepRight = !sweepRight; 
      sweepTimer = millis();
    }

    // Делаем четкие прерывистые рывки на месте для сканирования зон
    if (sweepRight) {
      turnRight(SEARCH_SPEED);
      delay(60); // Время микро-шага. Можно менять (35-55 мс)
      stopMotors();
    } else {
      turnLeft(SEARCH_SPEED);
      delay(60); 
      stopMotors();
    }
  }

  delay(20); // Пауза для корректной работы УЗ-датчиков
}
