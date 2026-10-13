#include <cstdint>

struct LoopMeasurement
{
  uint32_t previousTime = 0;
  uint32_t totalTime = 0;
  uint16_t ind = 0;
};

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