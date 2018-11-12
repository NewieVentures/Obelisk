#include "Particle.h"
#include "events.h"
#include "statusLed.h"

static void handleConnectivityEvent(system_event_t event, int data)
{
  switch(data) {
    case cloud_status_disconnecting:
    case cloud_status_disconnected:
    case network_status_powering_off:
    case network_status_off:
    case network_status_on:
    case network_status_powering_on:
      statusLed::rapidFlash();
      break;

    case network_status_connecting:
      statusLed::flash();
      break;

    case network_status_connected:
    case cloud_status_connecting:
      statusLed::slowFlash();
      break;

    case cloud_status_connected:
      statusLed::blink();
      break;
  }
}

void events::setup() {
  System.on(network_status+cloud_status, handleConnectivityEvent);
}
