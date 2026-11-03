#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 4;

unsigned long counter = 0;

constexpr uint8_t POLLING_INTERVAL = 5;
constexpr uint8_t DEBOUNCE_TIME = 50;

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

void setup() {

  Serial0.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  bool initialState = digitalRead(BUTTON_PIN) == HIGH;

  lastRawState = initialState;
  candidateState = initialState;

  buttonState = initialState
    ? ButtonState::PRESSED
    : ButtonState::RELEASED;

  candidateSince = millis();
}

void loop() {

  const unsigned long currentTime = millis();

  // Опитування кожні 5 мс
  if (currentTime - lastPollingTime < POLLING_INTERVAL)
    return;

  lastPollingTime = currentTime;

  // HIGH = натиснута, LOW = відпущена
  bool rawState = digitalRead(BUTTON_PIN) == HIGH;

  // Виявлення зміни фізичного стану
  if (rawState != lastRawState)
  {
    lastRawState = rawState;
    candidateState = rawState;
    candidateSince = currentTime;
  }

  // Перевірка стабільності протягом 50 мс
  if (currentTime - candidateSince >= DEBOUNCE_TIME)
  {
    bool stablePressed = buttonState == ButtonState::PRESSED;

    if (candidateState != stablePressed)
    {
      buttonState = candidateState
        ? ButtonState::PRESSED
        : ButtonState::RELEASED;

      if (buttonState == ButtonState::PRESSED)
      {
        Serial0.printf("Button pressed! Count: %lu\n", ++counter);
      }
    }
  }
}
