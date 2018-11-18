# Project Faraday
### By NewieVentures

Remote control of LED strip lights via DMX over cellular.

## Hardware
### Artifacts
* FP1 - Motherboard
* FP2 - DMX expansion module
* FA1 - Assembled unit
* FA2 - Demonstration panel

## LED patterns
To update the pattern of an active device, the device ID and a valid access token is required:

`curl -v "https://api.particle.io/v1/devices/<DEVICE_ID>/<function>" -d access_token=<TOKEN> -d arg=<ARG>`

eg to set a green strobe at 1Hz:

`curl -v "https://api.particle.io/v1/devices/<your device ID here>/strobe" -d access_token=<your access token here> -d arg="1000,#00FF00"`

### Blink
Flash LEDs on and off. Duty cycle controls amount of time on vs off.
#### Arguments
`"<period (ms)>,<duty cycle (0-100)>,<colour-on>,<colour-off>"`

eg blink red 1s:

`POST /v1/devices/:deviceId/blink { "arg": "1000,50,#FF0000,#000000" }`

### Colour
Fixed colour on all leds
#### Arguments
`"<colour>"`

Eg all LEDS blue:

`POST /v1/devices/:deviceId/colour { "arg": "#0000FF" }`

### Gradient
Linear colour gradient
#### Arguments
`"<start-colour>,<end-colour>"`

eg gradient on all LEDS from red to green:

`POST /v1/devices/:deviceId/gradient { "arg": "#FF0000,#00FF00" }`

### Progress
Progress bar
#### Arguments
* Initial value: 0 - (# leds – 1)
* Final value: 1 - #leds
* Direction: 0 = first LED to last, 1 = last LED to first

`"<initial value>,<final value><increment (# leds)>,<increment delay (ms)>,<reset delay(ms)>,<direction>,<colour-on>,<colour-off>"`

eg Progress bar from 0 to 100 leds with 1s between increments, 5s pause at the final value
before repeating and starting from the first LED to the last, incrementing by 1 LED with
RED being on and black being off:

```
POST /v1/devices/:deviceId/progress
{ "arg": "0,100,1,1000,5000,0,#FF0000,#000000" }
```

### Snake
Block of LEDS moving along the strip.
#### Arguments
* Direction: 0 = first LED to last, 1 = last LED to first
`"<period (ms)>,<direction>,<length (# leds)>,<colour-on>,<colour-off>"`

eg Green block of 3 LEDS moving from first LED to last every 5s on a black background:

`POST /v1/devices/:deviceId/snake { "arg": "5000,0,3,#00FF00,#000000" }`

### Strobe
Flash LEDs
#### Arguments
`"<period (ms)>,<colour>"`

eg Blue flash at 1Hz

`POST /v1/devices/:deviceId/strobe { "arg": "1000,#0000FF" }`

## Firmware
The firmware is compiled using the particle cloud development tools (internet connection required).

### Setup
Install the particle CLI, see docs at
https://docs.particle.io/tutorials/developer-tools/cli/

### Compiling
To compile using the particle CLI:
`particle compile electron app --saveTo electron.bin`

### Flashing
To flash a binary to the module over USB, put it into programming mode then:
`particle flash --usb electron.bin`

### Unit tests
```
cd src
make tests && ./tests
```

### Structure
The firmware runs on a Particle Electron board, using their Device OS.  The major firmware modules are:
* cloudFunctions - functions registered with Particle's Device OS on boot and called via their cloud interface.
* LedStripDriver - generates colour values for each LED based on the pattern and settings.  The cloud functions change the pattern settings, while a timer in the RTOS calls the onTimerFired() method to process the new values.
* dmx - uses Serial1 to send the DMX packets (requires some low level override of the baud rate to send the break and mark-after-break at the start of the packet) and sends the NULL start code in each packet.