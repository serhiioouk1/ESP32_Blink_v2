#include <Arduino.h>

constexpr uint8_t PIN = 9;

constexpr uint64_t WORK_INTERVAL = 2000000;
constexpr uint64_t OUTER_INTERVAL = 5000000;

hw_timer_t *timerWork = nullptr;
hw_timer_t *timerOuter = nullptr;

volatile bool timerOuterTriggered = false;
volatile bool timerWorkTriggered = false;

void IRAM_ATTR onTimerWork() {
  timerWorkTriggered = true;
}

void IRAM_ATTR onTimerOuter() {
  timerOuterTriggered = true;
}

void setup() {
  Serial0.begin(115200);

  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);

  // Ініціалізація: таймер 0, дільник 80, напрямок вверх
  timerWork = timerBegin(0, 80, true);
  timerOuter = timerBegin(1, 80, true);

  // Приєднання функції обробки переривання
  timerAttachInterrupt(timerWork, &onTimerWork, true);
  timerAttachInterrupt(timerOuter, &onTimerOuter, true);

  // Встановлення значення для переривання (1,000,000 мкс = 1 сек)
  timerAlarmWrite(timerWork, WORK_INTERVAL, false);
  timerAlarmWrite(timerOuter, OUTER_INTERVAL, true);

  // Включення переривання
  timerAlarmEnable(timerOuter);
}

void loop() {
  if(timerOuterTriggered) {

    Serial0.println("Outer timer triggered!");
    
    timerWrite(timerWork, 0); // Скидання таймера
    timerAlarmEnable(timerWork);
    
    digitalWrite(PIN, HIGH);
    
    timerOuterTriggered = false;
  }

  if(timerWorkTriggered) {
    
    Serial0.println("Work timer triggered!");

    digitalWrite(PIN, LOW);
    timerWorkTriggered = false;
  }
}
