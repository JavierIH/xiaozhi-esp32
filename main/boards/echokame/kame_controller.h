#ifndef _KAME_CONTROLLER_H_
#define _KAME_CONTROLLER_H_

#include <driver/gpio.h>

void InitializeKameController(gpio_num_t tx_pin);
void KameSendEmotion(const char* emotion);

// Forwards every emotion shown on the screen to the robot
template <typename BaseDisplay>
class KameEmotionDisplay : public BaseDisplay {
public:
    using BaseDisplay::BaseDisplay;

    bool SetEmotion(const char* emotion) override {
        KameSendEmotion(emotion);
        return BaseDisplay::SetEmotion(emotion);
    }
};

#endif  // _KAME_CONTROLLER_H_
