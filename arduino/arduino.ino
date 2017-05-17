// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels
#define PIN            5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      39

//Define the baud rate of the Arduino
#define BAUD 9600

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 0; // delay for per pixel turning on, 0 for instant

int red = 0;
int green = 0;
int blue = 155;

int maxBrightness = 255; //Maximum brightness scaler

void setup() {

  Serial.begin(BAUD);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
// End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {

 /* red = promptInt("r?");
  green = promptInt("g?");
  blue = promptInt("b?");
*/

  while(Serial.available()==0) //Wait for user input or serial port to initialize
  {

   } 

  red = Serial.readStringUntil(',').toInt();
  Serial.read(); //next character is comma, so skip it using this
  green = Serial.readStringUntil(',').toInt();
  Serial.read();
  blue = Serial.readStringUntil('\0').toInt();

  red = checkBounds(red);
  green = checkBounds(green);
  blue = checkBounds(blue);
    

    //parse your data here. example:
    //double x = Double.parseDouble(first);
  ardprintf("\n\nDone, set %d, %d, %d \n\n\n", red, green, blue);

  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.

  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(red,green,blue)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(delayval); // Delay for a period of time (in milliseconds).
  }
}

int checkBounds(int val){
  if(val > maxBrightness){
       val = maxBrightness;
   }

   if(val < 0){
      val = 0;
   }

   return val; //Return a parsed integer
}

int promptInt(String prompt){
  int val;
  Serial.println(prompt);
  
  

   val = Serial.parseInt();



} 

//The following function is bullshit to make sure I can actually print multiple variables in the same line. Aparently implementing prinf correnctly over serial was too much work for the core team....

#ifndef ARDPRINTF
#define ARDPRINTF
#define ARDBUFFER 16
#include <stdarg.h>
#include <Arduino.h>

int ardprintf(char *str, ...)
{
  int i, count=0, j=0, flag=0;
  char temp[ARDBUFFER+1];
  for(i=0; str[i]!='\0';i++)  if(str[i]=='%')  count++;

  va_list argv;
  va_start(argv, count);
  for(i=0,j=0; str[i]!='\0';i++)
  {
    if(str[i]=='%')
    {
      temp[j] = '\0';
      Serial.print(temp);
      j=0;
      temp[0] = '\0';

      switch(str[++i])
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
      j = (j+1)%ARDBUFFER;
      if(j==0) 
      {
        temp[ARDBUFFER] = '\0';
        Serial.print(temp);
        temp[0]='\0';
      }
    }
  };
  Serial.println();
  return count + 1;
}
#undef ARDBUFFER
#endif
