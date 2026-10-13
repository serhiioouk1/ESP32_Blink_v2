#include <cstdint>

struct GpioConfig
{
  static constexpr uint8_t BUTTON_READ_PIN = 11;
  static constexpr uint8_t LED_PIN = 16;
  static constexpr uint16_t blinkInterval = 500;
  static constexpr uint8_t DEBOUNCE_MS = 60;
};

enum BlinkMode
{
  ALWAYS_ON,
  ALWAYS_OFF,
  BLINK
};

enum class LedState 
{
  ON,
  OFF
};

class Led{

  public:    
    void init(const GpioConfig& config);
    void set(const GpioConfig& config, LedState state);
};