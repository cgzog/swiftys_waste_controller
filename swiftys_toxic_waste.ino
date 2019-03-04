// swiftys_toxic_waste
//
// Supports a configurable number of LEDs that each "throb" on and off with configurable on/off times
// for each LED
//
//
// Built using the 1.6.7 Arduino IDE
//
//
// Copyright (c) 2016, Christian Herzog
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be used
//    to endorse or promote products derived from this software without specific prior written
//    permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#define TRUE              1
#define FALSE             0


// LEDs have symbolic names but what is really important is the ports
#define LED_BLDG_GREEN    3
#define LED_BLDG_ORANGE   5

#define LED_POND_GREEN_1  6
#define LED_POND_GREEN_2  9
#define LED_POND_ORANGE   10
#define LED_POND_YELLOW   11

#define LED_HEARTBEAT     13    // Most Ardino boards have a built-in LED on pin 13
                                // we're using it as a heartbeat indicator so you can tell
                                // board is running if you have very long cycles times and
                                // the LED outputs are off because of where they are in their cycle
#define HEARTBEAT_CYCLE   500   // in milliseconds - on or off time - on 1/2 second - off 1/2 second

#define LED_MIN           2
#define LED_MAX           255


typedef struct {
  int   port;
  int   cycle;    // full cycle in milliseconds - on / off are each half of this time
  int   now;      // time in millis within current cycle (internal variable per LED, not configured)
} LED;

LED leds[] = {

  LED_BLDG_GREEN,   4000, 0,
  LED_BLDG_ORANGE,  1200, 0,
  LED_POND_GREEN_1, 7100, 0,
  LED_POND_GREEN_2, 8550, 0,
  LED_POND_ORANGE,  5580, 0,
  LED_POND_YELLOW,  4450, 0,
  0, 0, 0 };
 


void setup() {

int i;

  for (i = 0 ; leds[i].port != 0 ; i++) {
    pinMode(leds[i].port, OUTPUT);
    analogWrite(leds[i].port, LED_MIN);     // start with LEDs at minimum

    leds[i].now = 0;
  } 

  pinMode(LED_HEARTBEAT, OUTPUT);
  digitalWrite(LED_HEARTBEAT, HIGH);
}



void loop() {

int start = millis();
int now, old_now = start;
int i, heartbeat, heartbeatCount;
float percent;

  heartbeat      = TRUE;
  heartbeatCount = 0;

  while (TRUE) {

   now = millis() - start;             // time in millisecods since main loop started (LED cycle "epoch" time)
   
   // depending on where we are in the cycle for a specific LED (first half or second half)
   // we're either brightening or dimming the LED respectively
   //
   // we get an epoch time when we start the main loop and use that to calculate where a specific LED
   // is in it's cycle and adjust the brightness accordingly
   //
   // by recalculating where we are in the LED cycle each time through, we update the LED PWM in as fine an
   // increment as is available which is independent of how quickly we cycle through the LEDs - the faster we can go,
   // the finer the steps but we always catch up to the LED cycle rate

   for (i = 0 ; leds[i].port != 0 ; i++) {

      leds[i].now += now - old_now;
      
      if (leds[i].now > leds[i].cycle) {
        leds[i].now = 0;          // we've gone over the cycle so reset to 0 to restart
      } 

      if (leds[i].now <= (leds[i].cycle / 2)) {                                // first or second half of the cycle?
        percent = float(leds[i].now) / float(leds[i].cycle) * 2.0;             // first - brightening up the LED
      } else {
        percent = (1.0 - (float(leds[i].now) / float(leds[i].cycle))) * 2.0;   // second - dimming down the LED
      }

      analogWrite(leds[i].port, max(LED_MIN, percent * LED_MAX));              // set the LED brightness as calculated with LED_MIN floor
    } 

  heartbeatCount += now - old_now;              // get the time elapsed since the last go around
  
  if (heartbeatCount > HEARTBEAT_CYCLE) {       // time to update the heartbeat LED?
    if (heartbeat) {                            // toggle it
      digitalWrite(LED_HEARTBEAT, LOW);
      heartbeat = FALSE;
    } else {
      digitalWrite(LED_HEARTBEAT, HIGH);
      heartbeat = TRUE;
    }
    heartbeatCount = 0;
  }

  // save the current "now" tick count
  
  old_now = now;

  }
}
