#pragma once

#include "DisplaySegments.h"
#include "Thermostat.h"

class HeatDisplay
{
  public:
    HeatDisplay(int pinClk, int pinDio, PersistedData * storage, Thermostat * thermostat)
      : _display(pinClk, pinDio)
      , _storage(storage)
      , _thermostat(thermostat)
      , _celsius(storage->get_Celsius())
      , _brightness(storage->get_LedBrigtness())
      , _displayOn(storage->get_LedOn())
      , _configModeTimeStamp(0)
      , _configModeDimmer(false)
      , _blinkOff(false)
    {
      _display.setBrightness(_brightness);
    }

    void TestBrightness(unsigned long msDelay)
    {
      // We first go up to max brightness.
      for ( DisplaySegments::Brightness brightness = DisplaySegments::Brightness::LedMin; brightness <= DisplaySegments::Brightness::LedMax; brightness = brightness + 1 )
      {
        _display.setBrightness(brightness);
        _display.showNumberDec(8888);
        delay(msDelay);
      }

      // Then go back to where we want to end (or 1 from it).  Careful to not wrap the unsigned int endlessly.
      for ( DisplaySegments::Brightness brightness = DisplaySegments::Brightness::LedMax - 1; brightness > _brightness; brightness = brightness - 1 )
      {
        _display.setBrightness(brightness);
        _display.showNumberDec(8888);
        delay(msDelay);
      }

      // Since we go to just before the set brightness to avoid endless loop,
      // we need to execute this one more time.
      _display.setBrightness(_brightness);
      _display.showNumberDec(8888);
      delay(msDelay);

      if ( !_displayOn )
      {
        // This should show 'off' briefly.
        ShowBrightness();
        delay(msDelay);
      }

      _display.clear();
    }

    bool DisplayMessage(char * text, unsigned long msDelay)
    {
      return _display.scrollText(text, msDelay);
    }

    void ChangeConfigUp()
    {
      // If we've already entered config mode, then we can adjust the temp.
      if ( _configModeTimeStamp )
      {
        if ( _configModeDimmer )
        {
          if ( !_displayOn )
          {
            _displayOn = true;
            _storage->set_LedOn(_displayOn);
          }
          else if ( _brightness < DisplaySegments::Brightness::LedMax )
          {
            _brightness = _brightness + 1;
            _displayOn = true;
            _storage->set_LedBrigtness(_brightness);
            _storage->set_LedOn(_displayOn);
          }
          else
          {
            ConfigLimit();
          }
        }
        else
        {
          int oldTemp = _thermostat->GetTriggerTemp(_celsius);
          int newTemp = _thermostat->IncTriggerTemp(_celsius);
          if ( oldTemp == newTemp )
          {
            ConfigLimit();
          }
        }
      }
      _configModeTimeStamp = millis();
      UpdateDisplay();
    }

    void ChangeConfigDown()
    {
      // If we've already entered config mode, then we can adjust the temp.
      if ( _configModeTimeStamp )
      {
        if ( _configModeDimmer )
        {
          if ( _brightness > DisplaySegments::Brightness::LedMin )
          {
            _brightness = _brightness - 1;
            _storage->set_LedBrigtness(_brightness);
          }
          else if ( _displayOn )
          {
            _displayOn = false;
            _storage->set_LedOn(_displayOn);
          }
          else
          {
            ConfigLimit();
          }
        }
        else
        {
          int oldTemp = _thermostat->GetTriggerTemp(_celsius);
          int newTemp = _thermostat->DecTriggerTemp(_celsius);
          if ( oldTemp == newTemp )
          {
            ConfigLimit();
          }
        }
      }
      _configModeTimeStamp = millis();
      UpdateDisplay();
    }

    void ChangeConfigMode()
    {
      _configModeTimeStamp = millis();
      _configModeDimmer = !_configModeDimmer;
    }

    void ChangeMeasurement()
    {
      _celsius = !_celsius;
      _storage->set_Celsius(_celsius);
      UpdateDisplay();
    }

    void UpdateHeatCelsius(int tempCelsius)
    {
      // We only want to actually display the change if we aren't in the middle of changing config.
      if ( 0 == _configModeTimeStamp )
      {
        UpdateDisplay();
      }
    }

    void HandleBlink(unsigned long & delay)
    {
      // See if we are in config mode.
      if ( _configModeTimeStamp > 0 )
      {
        // We want to do the opposite of current state.
        if ( _blinkOff )
        {
          // Updating the display will also turn blink off.
          delay = _blinkIntervalOn;
          UpdateDisplay();
        }
        else
        {
          // For a smoother experience, it would be best to ensure if the use pressed
          // a button that we don't blink it off till at least the blink on interval.
          unsigned long elapsed = millis() - _configModeTimeStamp;
          if ( elapsed < _blinkIntervalOn )
          {
            // Set the callback to when we want to blink off.
            delay = _blinkIntervalOn - elapsed;
          }
          else
          {
            // If we are blinking to off, then clear display set blink off interval.
            _blinkOff = true;
            delay = _blinkIntervalOff;
            _display.clear();
          }
        }
      }
      else
      {
        // If we are not in config mode, the nothing to blink.
        delay = _blinkIntervalOn;
      }
    }

    void ConfigLimit()
    {
      _display.clear();
      delay(_configLimitBlinkOff);
    }

  private:
    void UpdateDisplay()
    {
      // Anytime we update the display, indicate that we aren't currently off for blinking
      // since we are about the update the display and show something.
      _blinkOff = false;

      // If we are in config mode, see if we should time out of it.
      if ( _configModeTimeStamp > 0 )
      {
        // Check how long since the user did something in config mode.
        // If we are timing out of config mode, show the current temp again.  
        unsigned long configLen = millis() - _configModeTimeStamp;
        if ( configLen > _configTimeOut )
        {
          // Reset us out of config mode.
          _configModeTimeStamp = 0;
          _configModeDimmer = false;
        }
      }

      // If we aren't in config mode, just show the temp.
      if ( 0 == _configModeTimeStamp )
      {
        _display.setBrightness(_brightness, _displayOn);
        if ( _displayOn )
        {
          ShowTemp(_thermostat->GetCurrentTemp(_celsius));
        }
        else
        {
          _display.clear();
        }
      }
      else
      {
        // This is config mode, so always show it even if the display is off.
        _display.setBrightness(_brightness);
        if ( _configModeDimmer )
        {
          ShowBrightness();
        }
        else
        {
          ShowTemp(_thermostat->GetTriggerTemp(_celsius));
        }
      }
    }

    void ShowTemp(int tempDisplay)
    {
      if ( Thermostat::IsErr(tempDisplay) )
      {
        _display.showText("Err");
        _display.showChar(DisplaySegments::Degree, DisplaySegments::Position::PosForth);
      }
      else if ( tempDisplay < 100 )
      {
        _display.showNumberDec((int)tempDisplay, false, 2);
        _display.showChar(DisplaySegments::Degree, DisplaySegments::Position::PosThird);
        _display.showChar(_celsius ? '[' : 'F', DisplaySegments::Position::PosForth);
      }
      else
      {
        _display.showNumberDec((int)tempDisplay, false, 3);
        _display.showChar(DisplaySegments::Degree, DisplaySegments::Position::PosForth);
      }
    }

    void ShowBrightness()
    {
      if ( _displayOn )
      {
        _display.showText("led");
        _display.showNumberDec((int)_brightness + 1, false, 1, DisplaySegments::Position::PosForth);
      }
      else
      {
        _display.showText("off");
      }
    }

  public:

    static const unsigned long BUTTON_LONG_PRESS = 2 /*seconds*/ * 1000;

  private:

    static const unsigned long _configTimeOut = 5 /*seconds*/ * 1000;
    static const unsigned long _configLimitBlinkOff = 100 /*ms*/;
    static const unsigned long _blinkIntervalOn = 500 /*ms*/;
    static const unsigned long _blinkIntervalOff = 500 /*ms*/;
    
    DisplaySegments _display;
    PersistedData * _storage;
    Thermostat * _thermostat;

    bool _celsius;
    DisplaySegments::Brightness _brightness;
    bool _displayOn;
    unsigned long _configModeTimeStamp;
    bool _configModeDimmer;
    bool _blinkOff;
};

