#pragma once

#include "ArduinoHandler.h"

class ButtonPress
{
  public:

    ButtonPress(int pinButton)
      : _pin(pinButton)
      , _longPressTime(0)
      , _pressTimeStamp(0)
      , _changeTimeStamp(0)
      , _pressedLastTime(false)
      , _pressHandled(false)
    {
      pinMode(pinButton, INPUT_PULLUP);
    }

    virtual ~ButtonPress()
    {
    }

    // Could consider making this part of the constructor since it doesn't make sense to have this w/o a short press handler.
    template <typename T>
    void RegisterShortPressHandler(T* obj, void (T::*method)())
    {
      _handlerShortPress.Register(obj, method);
    }

    template <typename T>
    void RegisterLongPressHandler(T* obj, void (T::*method)(), unsigned long pressMS)
    {
      _handlerLongPress.Register(obj, method);
      _longPressTime = pressMS;
    }

    void CheckButton(unsigned long & delay)
    {
      delay = CheckButtonPress();
    }

  private:

    unsigned long CheckButtonPress()
    {
      bool pressed = (digitalRead(_pin) == LOW);
      unsigned long now = millis();

      // We want to make sure we get the same state a few checks in a row before we act on it.
      if ( pressed != _pressedLastTime )
      {
        _pressedLastTime = pressed;
        _changeTimeStamp = now;
        return _minChangeTime;
      }

      // Same state as last time, so lets make sure we've passed the min time we should get the same reading.
      // If not, then we should bail out.
      unsigned long sinceLastChange = now - _changeTimeStamp;
      if ( sinceLastChange < _minChangeTime )
      {
        return _minChangeTime - sinceLastChange;
      }

      // At this point we should be sure this is a real change of state.
      if ( pressed )
      {
        // We're we previously in a pressed state.
        if ( 0 == _pressTimeStamp )
        {
          // Button in the pressed for the first time since we don't have a timestamp.
          // Just capture this and return.
          _pressTimeStamp = _changeTimeStamp;
          _pressHandled = false;  // This should already be in this state, but just to be sure.
          return _minChangeTime;
        }

        // We may have already handled the long press.  If so, we won't do anything until the state changes.
        if ( _pressHandled )
        {
          return _minChangeTime;
        }
        
        // We only need to check for long press if there is a handler for it.
        if ( _handlerLongPress.HasHandler() )
        {
          // Since the button is still pressed from a previous time, lets just check if
          // we've exceeded the long press time.  If so, we can return that state now.
          unsigned long pressLen = now - _pressTimeStamp;
          if ( pressLen > _longPressTime )
          {
            // Long Press
            _pressHandled = true;
            _handlerLongPress.Invoke();
            return _minChangeTime;
          }
        }
        // else we may want to consider handling the short press instantly if there is no long press handler.
        // If we don't, then the short press isn't handled until the user lets off.

        // Still held down, so nothing to do for this one either.
        return _minChangeTime;
      }

      // Button isn't pressed and if it wasn't pressed previously, there is nothing to do.
      if ( 0 == _pressTimeStamp )
      {
        _pressHandled = false;  // This should already be in this state, but just to be sure.
        return _minChangeTime;
      }

      // If the long press already handled this, we can simply reset the pressed state
      // and return since it was already handled and we were just waiting for the button to release.
      if ( _pressHandled )
      {
        _pressTimeStamp = 0;
        _pressHandled = false;
        return _minChangeTime;
      }

      // This should be a short press.  We've already ensured that it was in this state for the min
      // time at the top, so just do it already.
      _pressTimeStamp = 0;
      _handlerShortPress.Invoke();
      return _minChangeTime;
    }
 
  private:

    // This is the minimum time the button has to be pressed and registered in the same state before we'll act on it.
    // This will help with bouncing and ignoring quick transitions in states.  It is also the frequency we request to
    // be called to check our state.  We may have to make that frequency shorter than minimum button press if we see issues.
    static const unsigned long _minChangeTime = 50 /*ms*/;

    const int _pin;

    ArduinoHandler _handlerShortPress;
    ArduinoHandler _handlerLongPress;
    unsigned long _longPressTime;

    unsigned long _pressTimeStamp;
    unsigned long _changeTimeStamp;
    bool _pressedLastTime;
    bool _pressHandled;
};

