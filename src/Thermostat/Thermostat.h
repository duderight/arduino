#pragma once

#include <DHT11.h>  // https://github.com/dhrubasaha08/DHT11
#include "PersistedData.h"
#include "ArduinoHandler.h"

class Thermostat
{
  public:

    Thermostat(int pinSensor, PersistedData * storage)
      : _dht11(pinSensor)
      , _storage(storage)
      , _currentTempCelsius(ERROR_INIT)
      , _triggerTempFahrenheit(storage->get_ThermostatTemp())
    {
    }

    virtual ~Thermostat()
    {
    }

    template <typename T>
    void RegisterTempHandler(T* obj, void (T::*method)(int))
    {
      _handlerDisplay.Register(obj, method);

      // Just for completeness, we should try to notify the display the current temp
      // right when they register so they don't have to wait for the temp to change.
      // In practice, this won't do much since we register right away before we've got
      // a temp.  I don't want to actually read the temp here since that would happen
      // twice on startup, first now and then again the first time the worker calls us.
      if ( !IsErr(_currentTempCelsius) )
      {
        _handlerDisplay.Invoke(_currentTempCelsius);
      }
    }

    template <typename T>
    void RegisterRelayHandler(T* obj, void (T::*method)(bool))
    {
      _handlerRelay.Register(obj, method);

      // We want to refresh the relay right away when it first registers.
      // If we don't have our temp yet, this won't do anything but then we'll
      // get it the first time we do setup the temp, so no big deal.
      RefreshRelay();
    }

    void RefreshTemp(unsigned long & delay)
    {
      RefreshTemp();
      delay = _refreshInterval;
    }

    int GetCurrentTemp(bool celsius)
    {
      return celsius ? _currentTempCelsius : ConvertCtoF(_currentTempCelsius);
    }

    int GetTriggerTemp(bool celsius)
    {
      return celsius ? ConvertFtoC(_triggerTempFahrenheit) : _triggerTempFahrenheit;
    }

    int IncTriggerTemp(bool celsius)
    {
      return ChangeTriggerTemp(celsius, 1);
    }

    int DecTriggerTemp(bool celsius)
    {
      return ChangeTriggerTemp(celsius, -1);
    }

    static bool IsErr(int temp)
    {
      return (temp >= DHT11::ERROR_TIMEOUT);
    }

  private:

    void RefreshTemp()
    {
      const int lastTemp = _currentTempCelsius;
      _currentTempCelsius = _dht11.readTemperature();
      if ( lastTemp != _currentTempCelsius )
      {
        _handlerDisplay.Invoke(_currentTempCelsius);
        RefreshRelay();
      }
    }

    void RefreshRelay()
    {
      if ( !IsErr(_currentTempCelsius) )
      {
        // We always tell the relay what its state should be when the temp
        // changes and let it decide if it needs to do anything.
        bool relayOn = _currentTempCelsius >= ConvertFtoC(_triggerTempFahrenheit);
        _handlerRelay.Invoke(relayOn);
      }
    }

    int ChangeTriggerTemp(bool celsius, int amount)
    {
      int newTempF = celsius ? ConvertCtoF(ConvertFtoC(_triggerTempFahrenheit) + amount) : _triggerTempFahrenheit + amount;
      int oldTempF = _triggerTempFahrenheit;
      _triggerTempFahrenheit = constrain(newTempF, _minTriggerTempFahrenheit, _maxTriggerTempFahrenheit);
      if ( oldTempF != newTempF )
      {
        _storage->set_ThermostatTemp(_triggerTempFahrenheit);
        RefreshRelay();
      }
      return celsius ? ConvertFtoC(_triggerTempFahrenheit) : _triggerTempFahrenheit;
    }

    static int ConvertCtoF(int celsius)
    {
      return (celsius * 9 + 160) / 5;
    }

    static int ConvertFtoC(int fahrenheit)
    {
      return ((fahrenheit - 32) * 5 + 4) / 9;
    }

  public:

    static const int ERROR_INIT = 255;
 
  private:

    // The range of the DHT11 sensor is supposed to be from 0c (32f) to 50c (122f)
    static const int _maxTriggerTempFahrenheit = 122;
    static const int _minTriggerTempFahrenheit = 32;
    
    static const unsigned long _refreshInterval = 30 /*second*/ * 1000;

    DHT11 _dht11;

    PersistedData * _storage;

    ArduinoHandlerParam<int> _handlerDisplay;
    ArduinoHandlerParam<bool> _handlerRelay;

    // The sensor uses celsius, so we store it that way.
    int _currentTempCelsius;

    // But the user might want to see it as fahrenheit which is more granular,
    // so we store the trigger as fahrenheit so the user doesn't see weird jumps.
    int _triggerTempFahrenheit;
};

