#include <cstdint>

struct GpioConfig
{
  static constexpr uint8_t LED_PIN = 16;
  static constexpr uint16_t blinkInterval = 500;
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