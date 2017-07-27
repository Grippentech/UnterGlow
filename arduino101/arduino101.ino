#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <CurieBLE.h>

//Neopixel Pin
#define PIN 9

//Number of Neopixels Attached (experiment to find correct value or conut)
#define NUMPIXELS 39

//Define the baud rate of the Arduino
#define BAUD 9600

//Setup the neopixel library
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

char state; // variable to hold a transmitted byte

int delayval = 1; //delay for per pixel turning on, 0 for starters

//Values to hold a general color
int red = 0; 
int green = 0;
int blue = 0;

#define EFFECTDELAY 20 //Basic effect delay for use later

int currentEffect = 0; //Remember what the current effect is
int effectDelay = EFFECTDELAY; //Milliseconds

int maxBrightness = 255; //Maximum brightness scaler, adjust depending on your needs

BLEPeripheral blePeripheral;  //BLE Peripheral Device

// ====  create Nordic Semiconductor UART service =========
BLEService uartService = BLEService("6E400001B5A3F393E0A9E50E24DCCA9E");
// create characteristics
BLECharacteristic rxCharacteristic = BLECharacteristic("6E400002B5A3F393E0A9E50E24DCCA9E", BLEWriteWithoutResponse, 20);  // == TX on central (android app)
BLECharacteristic txCharacteristic = BLECharacteristic("6E400003B5A3F393E0A9E50E24DCCA9E", BLENotify , 20); // == RX on central (android app)

boolean wasRun = false; //Part of some spaghetti code in the loop that will need to be taken out when I get around to rewriting and testing something proper

void setup() {

  Serial.begin(BAUD);
  Serial.println("Ready.");

  pixels.begin(); // initialize the NeoPixel library.

  setAll(red, green, blue, delayval); //When Arduino starts up clear the array;

  // set advertised local name and service UUID:
  blePeripheral.setLocalName("GrippyDesk_BLE");
  blePeripheral.setAdvertisedServiceUuid(uartService.uuid());

  // add service, rx and tx characteristics:
  blePeripheral.addAttribute(uartService);
  blePeripheral.addAttribute(rxCharacteristic);
  blePeripheral.addAttribute(txCharacteristic);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handler for characteristic
  rxCharacteristic.setEventHandler(BLEWritten, rxCharacteristicWritten);

  // advertise the service
  blePeripheral.begin();
}

void loop() {
  switch (currentEffect) {
    case 0:
      wasRun = false; //This whole wasRun thing is spaghetti code, I know... 
      red = Serial.readStringUntil(',').toInt();
      Serial.read(); //next character is comma, so skip it using this
      green = Serial.readStringUntil(',').toInt();
      Serial.read();
      blue = Serial.readStringUntil('\0').toInt();
      red = checkBounds(red);
      green = checkBounds(green);
      blue = checkBounds(blue);
      if (wasRun = false) {
        setAll(red, green, blue, delayval);
        ardprintf("\n\nDone, set %d, %d, %d \n\n\n", red, green, blue);
      }
      wasRun = true;
      break;

    case 1:
      colorWipe(pixels.Color(255, 0, 0), effectDelay); // Red
      Serial.println(currentEffect);
      break;

    case 2:
      colorWipe(pixels.Color(0, 255, 0), effectDelay); // Green
      Serial.println(currentEffect);
      break;

    case 3:
      colorWipe(pixels.Color(0, 0, 255), effectDelay); // Blue
      Serial.println(currentEffect);
      break;

    case 4:
      theaterChase(pixels.Color(255, 0, 0), effectDelay); // Red
      Serial.println(currentEffect);
      break;

    case 5:
      theaterChase(pixels.Color(0, 255, 0), effectDelay*2.5); // Green
      Serial.println(currentEffect);
      break;

    case 6:
      theaterChase(pixels.Color(255, 0, 255), effectDelay*2.5); // Green
      Serial.println(currentEffect);
      break;

    case 7:
      rainbowCycle(effectDelay);
      Serial.println(currentEffect);
      break;

    case 8:
      rainbow(effectDelay);
      Serial.println(currentEffect);
      break;

    case 9:
    rainbow(effectDelay); //Placeholder sicne the last one is uggo
//      theaterChaseRainbow(effectDelay);
      Serial.println(currentEffect);
      break;
  }

  //Serial.println("It's looping");
  /* red = promptInt("r?");
    green = promptInt("g?");
    blue = promptInt("b?");
  */

  blePeripheral.poll(); //We can keep polling bluetooth if we're not getting Serial input


}

void setAll(int red, int green, int blue, int delayVal) {
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(red, green, blue)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

    //delay(delayval); // Delay for a period of time (in milliseconds).
  }
}

int checkBounds(int val) {
  if (val > maxBrightness) {
    val = maxBrightness;
  }

  if (val < 0) {
    val = 0;
  }

  return val; //Return a parsed integer
}

int promptInt(String prompt) {
  int val;
  Serial.println(prompt);
  val = Serial.parseInt();
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  //Serial.println("LED on");
  red = 125;
  green = 125;
  blue = 125;
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  //Serial.println("LED off");
  red = 0;
  green = 0;
  blue = 0;
}

void rxCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written: ");

  if (characteristic.value()) {       //null pointer check
    state = *characteristic.value();  //set state to be the value written from the phone/tablet to the Arduino 101
    Serial.println(char(state));      //print out the character to the serial monitor
    switch (char(state)) {
      case '1': //R+ button
        red += 20;
        break;

      case '2': //G+ button
        green += 20;
        effectDelay -= 20;
        Serial.println(effectDelay);
        break;

      case '3': //B+ Button
        blue += 20;
        break;

      case '4': //R- button
        red -= 20;
        break;

      case '5': //G- button
        green -= 20;
        effectDelay += 20;
        Serial.println(effectDelay);
        break;

      case '6': //B- Button
        blue -= 20;
        break;

      case 'c': //Stop Button
        red = 0;
        green = 0;
        blue = 0;
        currentEffect = 0;
        effectDelay = EFFECTDELAY;
        delayval = 1;
        setAll(red, green, blue, delayval);
        break;

      case 'f': //Previous Effect
        currentEffect--;
        if (currentEffect < 0) {
          currentEffect = 9; //If it tries to underflow go back to max
        }
        Serial.println(currentEffect);
        break;

      case 'g': //Next Effect
        currentEffect++;
        Serial.println(currentEffect);
        break;
    }

    if (currentEffect == 0) {
      setAll(red, green, blue, delayval); //Otherwise we induce spasms
    }

    if (effectDelay < 0 || effectDelay > 200) {
      effectDelay = EFFECTDELAY;
    }

    if (currentEffect > 9 || currentEffect < 0) {
      red = 0;
      green = 0;
      blue = 0;
      currentEffect = 0;
      effectDelay = EFFECTDELAY;
      delayval = 1;
      setAll(red, green, blue, delayval);
      Serial.println(currentEffect);
    }
  }
}

void changeColor(uint32_t c) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
  }
  pixels.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    delay(wait);
    pixels.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, c);  //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, 0);  //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, 0);  //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


//The following function is to make sure I can actually print multiple variables in the same line.

#ifndef ARDPRINTF
#define ARDPRINTF
#define ARDBUFFER 16
#include <stdarg.h>
#include <Arduino.h>

int ardprintf(char *str, ...)
{
  int i, count = 0, j = 0, flag = 0;
  char temp[ARDBUFFER + 1];
  for (i = 0; str[i] != '\0'; i++)  if (str[i] == '%')  count++;

  va_list argv;
  va_start(argv, count);
  for (i = 0, j = 0; str[i] != '\0'; i++)
  {
    if (str[i] == '%')
    {
      temp[j] = '\0';
      Serial.print(temp);
      j = 0;
      temp[0] = '\0';

      switch (str[++i])
      {
        case 'd': Serial.print(va_arg(argv, int));
          break;
        case 'l': Serial.print(va_arg(argv, long));
          break;
        case 'f': Serial.print(va_arg(argv, double));
          break;
        case 'c': Serial.print((char)va_arg(argv, int));
          break;
        case 's': Serial.print(va_arg(argv, char *));
          break;
        default:  ;
      };
    }
    else
    {
      temp[j] = str[i];
      j = (j + 1) % ARDBUFFER;
      if (j == 0)
      {
        temp[ARDBUFFER] = '\0';
        Serial.print(temp);
        temp[0] = '\0';
      }
    }
  };
  Serial.println();
  return count + 1;
}
#undef ARDBUFFER
#endif
