#include <Arduino.h>

class Pulser
{
public:
    bool on;
    Pulser(uint8_t pin)
    {
        value = 0;
        dir = 1;
        last = millis();
        on = false;
        this->pin = pin;
    }

    void pulse()
    {
        if (!on)
        {
            value = 0;
            dir = 1;
            analogWrite(this->pin, value);
            return;
        }

        unsigned long now = millis();
        if ((now - last) < 2)
        {
            return;
        }

        last = now;
        value += dir;
        if (value < 1)
        {
            dir = 1;
        }
        else if (value > 255)
        {
            dir = -1;
        }
        analogWrite(this->pin, value);
    }

private:
    int value;
    int dir;
    unsigned long last;
    uint8_t pin;
};