#pragma once

class RelayControl
{
  public:

    RelayControl(int pinOut)
      : _pin(pinOut)
    {
      pinMode(pinOut, OUTPUT);
    }

    virtual ~RelayControl()
    {
    }

    void ChangeState(bool enabled)
    {
      digitalWrite(_pin, enabled ? HIGH : LOW);
    }

  private:

    const int _pin;
};

