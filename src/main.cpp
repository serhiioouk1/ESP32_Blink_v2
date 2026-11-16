#include <Arduino.h>

constexpr uint8_t RED_PIN = 12;
constexpr uint8_t YELLOW_PIN = 10;
constexpr uint8_t GREEN_PIN = 7;
constexpr uint8_t BUTTON_PIN = 4;

constexpr uint8_t POLLING_INTERVAL = 5;
constexpr uint8_t DEBOUNCE_TIME = 50;

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

void pollingButton();
LightState getNextState(LightState currentState, bool buttonPressed);
void setNewBlinkTimerInterval(uint64_t newInterval);
void handleLeds(uint8_t red, uint8_t yellow, uint8_t green);

LightState state = LightState::Green;
unsigned long buttonLastInterruptTime = 0;

//button variables
enum class ButtonState
{
    RELEASED,
    PRESSED
};

ButtonState buttonState = ButtonState::RELEASED;

bool lastRawState = false;
bool candidateState = false;

unsigned long lastPollingTime = 0;
unsigned long candidateSince = 0;
unsigned long counter = 0;
bool buttonPressedEvent = false;

void setup() {
  Serial0.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //init green light first
  handleLeds(HIGH, LOW, LOW);

  //button setup
  bool initialState = digitalRead(BUTTON_PIN) == LOW;

  lastRawState = initialState;
  candidateState = initialState;

  buttonState = initialState
    ? ButtonState::PRESSED
    : ButtonState::RELEASED;

  candidateSince = millis();

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

  pollingButton();

  if (buttonPressedEvent) {
    
    if(state == LightState::YellowBlinking) {
      
      timerWrite(blinkTimer, 0);
      timerAlarmDisable(lightTimer);
      timerAlarmEnable(blinkTimer);
    }
    else {
      
      timerAlarmEnable(lightTimer);
      timerAlarmDisable(blinkTimer);
    }

    Serial0.printf("Button pressed! Count: %lu\n", ++counter);
  }

  if(blinkTimerTriggered || lightTimerTriggered || buttonPressedEvent) {
    
    if(blinkTimerTriggered || buttonPressedEvent) {
      
      state = getNextState(state, buttonPressedEvent);
      
      buttonPressedEvent = false;
    }

    switch( state ) {
        case LightState::Green:

            handleLeds(HIGH, LOW, LOW);
            
            setNewBlinkTimerInterval(GREEN_INTERVAL);
          break;
        case LightState::GreenBlinking:
        
          if(blinkTimerTriggered) {
            
            timerWrite(lightTimer, 0); // Скидання таймера
            timerAlarmEnable(lightTimer); // Увімкнення таймера для блимання

            setNewBlinkTimerInterval(GREEN_BLINKING_INTERVAL);
          }
          else if(lightTimerTriggered) {
            
            // Toggle the green LED state
            static bool isGreenOn = true;
            if (isGreenOn) {
              handleLeds(LOW, LOW, LOW); // Turn off green LED
            } else {
              handleLeds(HIGH, LOW, LOW); // Turn on green LED
            }
            
            isGreenOn = !isGreenOn;
          }
          break;
        case LightState::Yellow:

          handleLeds(LOW, HIGH, LOW);
          
          timerAlarmDisable(lightTimer);

          setNewBlinkTimerInterval(YELLOW_INTERVAL);
          break;
        case LightState::Red:
            
          handleLeds(LOW, LOW, HIGH);

          setNewBlinkTimerInterval(RED_INTERVAL);
          break;
        case LightState::RedYellow:
          
          handleLeds(LOW, HIGH, HIGH);
        
          setNewBlinkTimerInterval(RED_YELLOW_INTERVAL);
          break;
        case LightState::YellowBlinking:

          if(lightTimerTriggered) {
            
            static bool isYellowOn = true;
            if (isYellowOn) {
              handleLeds(LOW, LOW, LOW); // Turn off yellow LED
            } else {
              handleLeds(LOW, HIGH, LOW); // Turn on yellow LED
            }
            
            isYellowOn = !isYellowOn;
          }
          break;
      }

      blinkTimerTriggered = false;
      lightTimerTriggered = false;
    }
}

void pollingButton()
{
  const unsigned long currentTime = millis();

  // Опитування кожні 5 мс
  if (currentTime - lastPollingTime < POLLING_INTERVAL)
    return;

  lastPollingTime = currentTime;

  // HIGH = натиснута, LOW = відпущена
  bool rawState = digitalRead(BUTTON_PIN) == LOW;

  // Виявлення зміни фізичного стану
  if (rawState != lastRawState) {
    
    lastRawState = rawState;
    candidateState = rawState;
    candidateSince = currentTime;
  }

  // Перевірка стабільності протягом 50 мс
  if (currentTime - candidateSince >= DEBOUNCE_TIME) {
    
    bool stablePressed = buttonState == ButtonState::PRESSED;

    if (candidateState != stablePressed) {
      
      buttonState = candidateState
        ? ButtonState::PRESSED
        : ButtonState::RELEASED;
 
        buttonPressedEvent = buttonState == ButtonState::PRESSED;
    }
  }
}

LightState getNextState(LightState currentState, bool buttonPressed) {

  if (buttonPressedEvent) {
    if(state == LightState::YellowBlinking) {
      
      return LightState::Green;
    }
    else {
      
      return LightState::YellowBlinking;
    }
  }
  else {
    switch (currentState) {
        case LightState::Green:
            return LightState::GreenBlinking;
        case LightState::GreenBlinking:
            return LightState::Yellow;
        case LightState::Yellow:
            return LightState::Red;
        case LightState::Red:
            return LightState::RedYellow;
        case LightState::RedYellow:
            return LightState::Green;
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