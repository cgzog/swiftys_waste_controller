// Paint example specifically for the TFTLCD breakout board.
// If using the Arduino shield, use the tftpaint_shield.pde sketch instead!
//
// DOES NOT CURRENTLY WORK ON ARDUINO LEONARDO (but that isn't a big
// deal as TFTLCD breakout is formatted for the Uno anway)

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7

// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 22
//   D1 connects to digital pin 23
//   D2 connects to digital pin 24
//   D3 connects to digital pin 25
//   D4 connects to digital pin 26
//   D5 connects to digital pin 27
//   D6 connects to digital pin 28
//   D7 connects to digital pin 29

// For the Arduino Due, use digital pins 33 through 40
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 33
//   D1 connects to digital pin 34
//   D2 connects to digital pin 35
//   D3 connects to digital pin 36
//   D4 connects to digital pin 37
//   D5 connects to digital pin 38
//   D6 connects to digital pin 39
//   D7 connects to digital pin 40

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 40
#define PENRADIUS 3

#define LCD_HIGH    320
#define LCD_WIDE    240

#define LINE_WIDTH  30

#define TO_NORMAL   1     // turnout settings
#define TO_DIVERGE  2

#define OUT_TO_IN   1     // turnout IDS
#define IN_TO_OUT   2
#define SWIFTYS     3

#define MAINLINE_CTRL_X     85
#define OUT_TO_IN_CTRL_X    MAINLINE_CTRL_X
#define IN_TO_OUT_CTRL_X    MAINLINE_CTRL_X
#define SWIFTYS_CTRL_X      145

#define IN_TO_OUT_CTRL_Y   110
#define OUT_TO_IN_CTRL_Y   (LCD_HIGH - IN_TO_OUT_CTRL_Y)
#define SWIFTYS_CTRL_Y     (LCD_HIGH - 50)

#define TO_IND_RADIUS       20


// comm lines to maintain state between two modules
//
// the output lines from one module should connect to the H bridges to drive the Tortoises
//
// BE SURE TO SHARE A COMMON GND between the modules!

#define   IN_TO_OUT_IN    A5
#define   OUT_TO_IN_IN    2
#define   SWIFTYS_IN      3
#define   IN_TO_OUT_OUT   10
#define   OUT_TO_IN_OUT   11
#define   SWIFTYS_OUT     12



int OutToInState = TO_NORMAL;
int InToOutState = TO_NORMAL;
int SwiftysState = TO_NORMAL;

#define DEBOUNCE_DELAY  1000    // milliseconds between switch activations

#define MINPRESSURE 10
#define MAXPRESSURE 1000




void setTurnout(int turnout, int direction) {

int color;
int pinOut;

  switch (direction) {

    case TO_NORMAL:
      color = GREEN;
      pinOut = HIGH;    // HIGH is mainline
      break;

    case TO_DIVERGE:
      color = RED;
      pinOut = LOW;     // LOW is diverging
      break;
  }

  switch (turnout) {

    case OUT_TO_IN:
      tft.fillCircle(OUT_TO_IN_CTRL_X, OUT_TO_IN_CTRL_Y, TO_IND_RADIUS, color);
//      digitalWrite(OUT_TO_IN_OUT, pinOut);
      break;

    case IN_TO_OUT:
      tft.fillCircle(IN_TO_OUT_CTRL_X, IN_TO_OUT_CTRL_Y, TO_IND_RADIUS, color);
//      digitalWrite(IN_TO_OUT_OUT, pinOut);
      break;

    case SWIFTYS:
        tft.fillCircle(SWIFTYS_CTRL_X, SWIFTYS_CTRL_Y, TO_IND_RADIUS, color);
//        digitalWrite(SWIFTYS_OUT, pinOut);
        break;
  }
}


void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Start!"));
  
  tft.reset();
  
  uint16_t identifier = tft.readID();

  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    return;
  }

  tft.begin(identifier);

  tft.fillScreen(BLACK);

// mainlines
  tft.fillRect(40, 0, LINE_WIDTH, LCD_HIGH, BLUE);          // outside main
  tft.fillRect(100, 0, LINE_WIDTH, LCD_HIGH, BLUE);          // inside main
  tft.fillRect(175, 20, LINE_WIDTH, 200, BLUE);  // siding

// Crossovers (drawn with lot's of angled lines

  int i, xvrRight, xvrLeft;

  xvrRight = MAINLINE_CTRL_X - LINE_WIDTH / 2 - LINE_WIDTH;
  xvrLeft  = MAINLINE_CTRL_X + LINE_WIDTH / 2;     // out to in

  for (i = 0 ; i < 30 ; i++) {
    tft.drawLine(xvrRight, 10, xvrLeft, 100, BLUE);
    xvrRight++ ; xvrLeft++;
  }

  xvrRight = MAINLINE_CTRL_X - LINE_WIDTH / 2 - LINE_WIDTH;
  xvrLeft  = MAINLINE_CTRL_X + LINE_WIDTH / 2;     // in to out
  // xvrRight = 30 ; xvrLeft = 90;     // in to out

  for (i = 0 ; i < 30 ; i++) {
    tft.drawLine(xvrRight, LCD_HIGH - 10, xvrLeft, LCD_HIGH - 100, BLUE);
    xvrRight++ ; xvrLeft++;
  }
  
  xvrRight = 100 ; xvrLeft = 175;    // siding

  for (i = 0 ; i < 30 ; i++) {
    tft.drawLine(xvrRight, LCD_HIGH - 20, xvrLeft, 220, BLUE);
    xvrRight++ ; xvrLeft++;
  }

//  pinMode(OUT_TO_IN_IN, INPUT);
//  pinMode(IN_TO_OUT_IN, INPUT);
//  pinMode(SWIFTYS_IN, INPUT);

//  pinMode(OUT_TO_IN_OUT, OUTPUT);
//  pinMode(IN_TO_OUT_OUT, OUTPUT);
//  pinMode(SWIFTYS_OUT, OUTPUT);

  setTurnout(OUT_TO_IN, OutToInState);
  setTurnout(IN_TO_OUT, InToOutState);
  setTurnout(SWIFTYS, SwiftysState);
  
  pinMode(13, OUTPUT);
}

unsigned long now;
unsigned long outToInTime = 0;
unsigned long inToOutTime = 0;
unsigned long swiftysTime = 0;


void loop()
{
  now = millis();
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  // pinMode(YM, OUTPUT);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {

    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    p.x = LCD_WIDE - p.x; // Normalize to match graphic coordinates
    p.y = LCD_HIGH - p.y;
    
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.print(")  now = "); Serial.print(now);
    Serial.print(", swiftysTime = "); Serial.println(swiftysTime);

    // check to see if this is an active turnout control

    // OUT_TO_IN?

    if (p.x > (OUT_TO_IN_CTRL_X - TO_IND_RADIUS * 2) && p.x < (OUT_TO_IN_CTRL_X + TO_IND_RADIUS * 2) &&
        p.y > (OUT_TO_IN_CTRL_Y - TO_IND_RADIUS * 2) && p.y < (OUT_TO_IN_CTRL_Y + TO_IND_RADIUS * 2) &&
        now - outToInTime > DEBOUNCE_DELAY) {

       if (OutToInState == TO_NORMAL) {
         OutToInState = TO_DIVERGE;
       } else {
         OutToInState = TO_NORMAL;
       }

       setTurnout(OUT_TO_IN, OutToInState);
       outToInTime = now;
     }

    // IN_TO_OUT?

    if (p.x > (IN_TO_OUT_CTRL_X - TO_IND_RADIUS * 2) && p.x < (IN_TO_OUT_CTRL_X + TO_IND_RADIUS * 2) &&
        p.y > (IN_TO_OUT_CTRL_Y - TO_IND_RADIUS * 2) && p.y < (IN_TO_OUT_CTRL_Y + TO_IND_RADIUS * 2) &&
        now - inToOutTime > DEBOUNCE_DELAY) {

       if (InToOutState == TO_NORMAL) {
         InToOutState = TO_DIVERGE;
       } else {
         InToOutState = TO_NORMAL;
       }

       setTurnout(IN_TO_OUT, InToOutState);
       inToOutTime = now;
    }

    // SWIFTYS?

    if (p.x > (SWIFTYS_CTRL_X - TO_IND_RADIUS * 2) && p.x < (SWIFTYS_CTRL_X + TO_IND_RADIUS * 2) &&
        p.y > (SWIFTYS_CTRL_Y - TO_IND_RADIUS * 2) && p.y < (SWIFTYS_CTRL_Y + TO_IND_RADIUS * 2) &&
        now - swiftysTime > DEBOUNCE_DELAY) {

       if (SwiftysState == TO_NORMAL) {
         SwiftysState = TO_DIVERGE;
       } else {
         SwiftysState = TO_NORMAL;
       }

       setTurnout(SWIFTYS, SwiftysState);
       swiftysTime = now;
     }
  } 

/*
   delay(50);     // wait a bit and then check the input lines from the other controller and set the
                  // turnouts appropriately
                  //
                  // it should have picked up our changes and on the *_OUT lines and echoed them back
                  // on the *_IN lines or it could have been the source fo the changes and now we'll echo them back

  if (digitalRead(IN_TO_OUT_IN) == HIGH) {
    InToOutState = TO_DIVERGE;
  } else {
    InToOutState = TO_NORMAL;
  }

  setTurnout(IN_TO_OUT, InToOutState);    // set it unconditionally - it will either be the same (which would be benign)
                                          // or it will echo the change from the other module
  if (digitalRead(OUT_TO_IN_IN) == HIGH) {
    OutToInState = TO_DIVERGE;
  } else {
    OutToInState = TO_NORMAL;
  }

  setTurnout(OUT_TO_IN, OutToInState);


  if (digitalRead(SWIFTYS_IN) == HIGH) {
    SwiftysState = TO_DIVERGE;
  } else {
    SwiftysState = TO_NORMAL;
  }

  setTurnout(IN_TO_OUT, SwiftysState);
*/
}

