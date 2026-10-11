#include <cstdint>
enum class LedState 
{
  ON,
  OFF
};

class Led{

  private: 
    static constexpr uint8_t LED_PIN = 16;

  public:
    static constexpr uint16_t blinkInterval = 500;
    
    void init();
    void set(LedState state);
};