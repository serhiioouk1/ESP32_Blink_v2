#include <Arduino.h>

constexpr uint8_t RED_PIN = 12;
constexpr uint8_t YELLOW_PIN = 10;
constexpr uint8_t GREEN_PIN = 7;

//1000000; //1000000 = 1 second in microseconds
/*Green_INTERVAL = 6s;
Green_BLINKING_INTERVAL = 3s;
Yellow_INTERVAL = 4s;
RED_INTERVAL = 5s;
Red_Yellow_INTERVAL = 1s;*/
constexpr uint64_t GREEN_INTERVAL = 6000000;
constexpr uint64_t GREEN_BLINKING_INTERVAL = 3000000;
constexpr uint64_t YELLOW_INTERVAL = 4000000;
constexpr uint64_t RED_INTERVAL = 5000000;
constexpr uint64_t RED_YELLOW_INTERVAL = 1000000;

constexpr uint64_t Blinking_Frequency = 500000;

uint64_t CURRENT_INTERVAL = GREEN_INTERVAL;

hw_timer_t *blinkTimer = nullptr;
hw_timer_t *lightTimer = nullptr;

enum class LightState
{
  Green,
  GreenBlinking,
  Yellow,
  Red,
  RedYellow,
  YellowBlinking
};

volatile bool blinkTimerTriggered = false;
volatile bool lightTimerTriggered = false;

void IRAM_ATTR onBlinkTimerTrigger() {
  blinkTimerTriggered = true;
}

void onLightTimerTrigger() {
  lightTimerTriggered = true;
}

void setNewBlinkTimerInterval(uint64_t newInterval);
void handleLeds(uint8_t red, uint8_t yellow, uint8_t green);

LightState state = LightState::Green;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  //init green light first
  handleLeds(HIGH, LOW, LOW);

   // Ініціалізація: таймер 0, дільник 80, напрямок вверх
  blinkTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(blinkTimer, &onBlinkTimerTrigger, true);
  timerAlarmWrite(blinkTimer, GREEN_INTERVAL, true);
  timerAlarmEnable(blinkTimer);

  // Ініціалізація: таймер 1, дільник 80, напрямок вверх
  lightTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(lightTimer, &onLightTimerTrigger, true);
  timerAlarmWrite(lightTimer, Blinking_Frequency, true);
}

void loop() {

  if(lightTimerTriggered) {
    lightTimerTriggered = false;

    // Toggle the green LED state
    static bool isGreenOn = true;
    if (isGreenOn) {
      handleLeds(LOW, LOW, LOW); // Turn off green LED
    } else {
      handleLeds(HIGH, LOW, LOW); // Turn on green LED
    }
    
    isGreenOn = !isGreenOn;
  }

  if(blinkTimerTriggered) {
    blinkTimerTriggered = false;

    switch( state ) {
        case LightState::Green:
          
          timerWrite(lightTimer, 0); // Скидання таймера
          timerAlarmEnable(lightTimer); // Увімкнення таймера для блимання

          setNewBlinkTimerInterval(GREEN_BLINKING_INTERVAL);
          
          state = LightState::GreenBlinking;
          break;
        case LightState::GreenBlinking:
        
          handleLeds(LOW, HIGH, LOW);
          
          timerAlarmDisable(lightTimer);

          setNewBlinkTimerInterval(YELLOW_INTERVAL);

          state = LightState::Yellow;
          break;
        case LightState::Yellow:
          
          handleLeds(LOW, LOW, HIGH);

          setNewBlinkTimerInterval(RED_INTERVAL);
          
          state = LightState::Red;
          break;
        case LightState::Red:

          handleLeds(LOW, HIGH, HIGH);
          
          setNewBlinkTimerInterval(RED_YELLOW_INTERVAL);
          
          state = LightState::RedYellow;
          break;
        case LightState::RedYellow:
          
          handleLeds(HIGH, LOW, LOW);
          
          setNewBlinkTimerInterval(GREEN_INTERVAL);
          
          state = LightState::Green;
          break;
        /*case LightState::YellowBlinking:
          state = LightState::Red;
          break;*/
      }
    }
}

void handleLeds(uint8_t green, uint8_t yellow, uint8_t red)
{
  digitalWrite(GREEN_PIN, green);
  digitalWrite(YELLOW_PIN, yellow);
  digitalWrite(RED_PIN, red);
}

void setNewBlinkTimerInterval(uint64_t newInterval)
{
    if (CURRENT_INTERVAL == newInterval)
        return;

    timerAlarmDisable(blinkTimer);

    timerAlarmWrite(blinkTimer, newInterval, true);

    timerWrite(blinkTimer, 0);

    timerAlarmEnable(blinkTimer);

    CURRENT_INTERVAL = newInterval;
}