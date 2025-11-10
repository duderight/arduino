#include <Arduino.h>
#include "ArduinoWorker.h"
#include "PersistedData.h"
#include "Thermostat.h"
#include "RelayControl.h"
#include "HeatDisplay.h"
#include "ButtonPress.h"
#include "config.h"  // include last so no others use these directly

ArdunioWorker worker;
PersistedData storage;
Thermostat thermostat(PIN_HEAT_DIO, &storage);
RelayControl relay(PIN_RELAY);
HeatDisplay display(PIN_DISPLAY_CLK, PIN_DISPLAY_DIO, &storage, &thermostat);
ButtonPress buttonRed(PIN_BUTTON_RED);
ButtonPress buttonBlue(PIN_BUTTON_BLUE);

void setup()
{
  // The persisted storage object needs to be called to ensure it saves any config changes.
  worker.AddWorker(PASS_OBJECT_METHOD(storage, SaveData));

  // The thermostat needs to refresh the temp and notify the relay and display.
  thermostat.RegisterRelayHandler(PASS_OBJECT_METHOD(relay, ChangeState));
  thermostat.RegisterTempHandler(PASS_OBJECT_METHOD(display, UpdateHeatCelsius));
  worker.AddWorker(PASS_OBJECT_METHOD(thermostat, RefreshTemp));

  // Create a handler to blink the display when needed.
  worker.AddWorker(PASS_OBJECT_METHOD(display, HandleBlink));

  // The red (up) button need to be monitored for press and notify the display when the occur. 
  buttonRed.RegisterShortPressHandler(PASS_OBJECT_METHOD(display, ChangeConfigUp));
  buttonRed.RegisterLongPressHandler(PASS_OBJECT_METHOD(display, ChangeConfigMode), HeatDisplay::BUTTON_LONG_PRESS);
  worker.AddWorker(PASS_OBJECT_METHOD(buttonRed, CheckButton));

  // Same for the blue (down) button.
  buttonBlue.RegisterShortPressHandler(PASS_OBJECT_METHOD(display, ChangeConfigDown));
  buttonBlue.RegisterLongPressHandler(PASS_OBJECT_METHOD(display, ChangeMeasurement), HeatDisplay::BUTTON_LONG_PRESS);
  worker.AddWorker(PASS_OBJECT_METHOD(buttonBlue, CheckButton));

#ifdef STARTUP_MSG
  display.DisplayMessage(STARTUP_MSG, STARTUP_SPEED);
#endif

#ifdef BRIGHTNESS_TEST_DELAY
  display.TestBrightness(BRIGHTNESS_TEST_DELAY);
#endif
}

void loop()
{
  unsigned long timeToNextWorker = worker.RunWorkers();
  delay(timeToNextWorker);
}
