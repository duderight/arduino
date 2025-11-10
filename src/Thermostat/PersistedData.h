#pragma once

#include <EEPROM.h>

class PersistedData
{
  public:
    
    PersistedData()
      : _storageDirty(0)
    {
      EEPROM.get(_address, _storage);
      if ( ( _storage.sig != _sig) ||
           ( _storage.size != sizeof(_storage) ) )
      {
        _storage.sig = _sig;
        _storage.size = sizeof(_storage);
        _storage.flags = _defaultFlags;
        _storage.temp = _defaultTemp;
      }
    }

    void SaveData(unsigned long & delay)
    {
      // See if the dirty storage bit is set.
      if ( _storageDirty )
      {
        // Check to make sure our save freq time has elapsed since last change.
        // This ensures that if the user is still changing config that we don't
        // save until they are all done (or at least the normal config timeout is hit).
        unsigned long elapsed = millis() - _storageDirty;
        if ( elapsed < _saveFreq )
        {
          // Since the user has changed more recently than our save frequency, lets
          // wait till the time would be exactly the save frequency;
          delay = _saveFreq - elapsed;
          return;
        }

        // Otherwise if we didn't return, we should save the storage and reset the dirty bit.
        EEPROM.put(_address, _storage);
        _storageDirty = 0;
      }

      // Now do our normal save frequency check (because we just saved or there wasn't anything to save).
      delay = _saveFreq;
    }

    uint8_t get_LedBrigtness()
    {
      return (_storage.flags & MASK_BRIGHTNESS_VALUE);
    }

    void set_LedBrigtness(uint8_t led)
    {
      if ( get_LedBrigtness() != (led & MASK_BRIGHTNESS_VALUE) )
      {
        _storage.flags = (_storage.flags & ~MASK_BRIGHTNESS_VALUE) | (led & MASK_BRIGHTNESS_VALUE);
        _storageDirty = millis();
      }
    }

    bool get_LedOn()
    {
      return get_flag(FLAG_BRIGHTNESS_ON);
    }

    void set_LedOn(bool enabled)
    {
      set_flag(enabled, FLAG_BRIGHTNESS_ON);
    }

    bool get_Celsius()
    {
      return get_flag(FLAG_CELSIUS);
    }

    void set_Celsius(bool enabled)
    {
      set_flag(enabled, FLAG_CELSIUS);
    }

    uint8_t get_ThermostatTemp()
    {
      return _storage.temp;
    }

    void set_ThermostatTemp(uint8_t temp)
    {
      if ( _storage.temp != temp )
      {
        _storage.temp = temp;
        _storageDirty = millis();
      }
    }

  private:

    bool get_flag(uint8_t flag)
    {
      return (flag == (_storage.flags & flag));
    }

    void set_flag(bool enabled, uint8_t flag)
    {
      if ( get_flag(flag) != enabled )
      {
        if ( enabled )
        {
          _storage.flags |= flag;
        }
        else
        {
          _storage.flags &= ~flag;
        }
        EEPROM.put(_address, _storage);
      }
    }

  private:
    static const int _address = 0;
    static const int _sig = 0x04281976;

    // Generally it is a good idea to keep this the same as the _configTimeOut in HeadDisplay
    // so we try to save the settings just after the config times out and is "finished".
    static const unsigned long _saveFreq = 5 /*seconds*/ * 1000;

    static const uint8_t MASK_BRIGHTNESS_VALUE  = 0x07;
    static const uint8_t FLAG_BRIGHTNESS_ON     = 0x08;
    static const uint8_t FLAG_CELSIUS           = 0x10;

    static const uint8_t _defaultFlags = MASK_BRIGHTNESS_VALUE /*DisplaySegments::Brightness::LedMax*/ | FLAG_BRIGHTNESS_ON | FLAG_CELSIUS;
    static const uint8_t _defaultTemp = 86; // degree F
    
    struct Storage
    {
      int sig;
      size_t size;
      uint8_t flags;
      uint8_t temp;
    };

    Storage _storage;
    unsigned long _storageDirty;
};
