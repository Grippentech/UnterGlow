
int 
  ColorPickerX, //color picker horizontal position
  ColorPickerY, //color picker vertical position
  LineY, //hue line vertical position
  CrossX, //saturation+brightness cross horizontal position
  CrossY, //saturation+brightness cross horizontal position
  ColorSelectorX = -500, //color selector button horizontal position <------------------------------------------- CHANGE
  ColorSelectorY = 50; //color selector button vertical position   <------------------------------------------- CHANGE

boolean 
  isDraggingCross = false, //check if mouse is dragging the cross
  isDraggingLine = false, //check if mouse is dragging the line
  ShowColorPicker = false; //toggle color picker visibility (even = not visible, odd = visible) 

color 
  activeColor = color(100, 100, 100), //contain the selected color  
  interfaceColor = color(255); //change as you want               <------------------------------------------- CHANGE

import processing.serial.*;
import static javax.swing.JOptionPane.*;

int baud = 9600; 

Serial myPort;  // Create object from Serial class

final boolean debug = true; //Makes program verbose and prints helpful debug data in processing.

int old_red = 0;
int old_green = 0;
int old_blue = 0;

//GUI library for dropdown
import controlP5.*;
import java.util.*;
ControlP5 cp5;

//Predefined colors, should probably store as array long term but can't super be bothered rn.
String[] presets = new String[8];



void setup() 

{
  presets[0] = "115,2,2"; //Club Red;
  presets[1] = "0,255,0"; //Hulk Green;
  presets[2] = "125,0,0"; //Tomato Red;
  presets[3] = "0,123,167"; //Cyrulean;
  presets[4] = "0,128,128"; //Teal;
  presets[5] = "253,184,19"; //Sun Yellow;
  presets[6] = "255,255,255"; //White;
  presets[7] = "255,105,180"; //Hot Pink;
  
  size(350, 350);
  smooth();
  surface.setResizable(false); //This doesn't work properly rn but maybe in the future.

  colorMode(HSB);

  cp5 = new ControlP5(this);
  List l = Arrays.asList("Club Red", "Hulk Green", "Tomato Red", "Cyrulean", "Teal", "Sun Yellow", "White", "Hot Pink");
  /* add a ScrollableList, by default it behaves like a DropdownList */
  cp5.addScrollableList("Color Presets")
    .setPosition(0, 0)
    .setSize(width, 300)
    .setBarHeight(40)
    .setItemHeight(25)
    .addItems(l)
    //.moveTo("extra")
    .setType(ScrollableList.LIST) // currently supported DROPDOWN and LIST
    .setOpen(false)                      //false for closed
    ;

  ColorPickerX = constrain( ColorSelectorX + 40, 10, width - 340 ); //set color picker x position to color selector + 40 and avoid it to be out of screen 
  ColorPickerY = constrain( ColorSelectorY + 40, 10, height - 300 ); //set color picker y position to color selector + 40 and avoid it to be out of screen 

  LineY = ColorPickerY + int(hue(activeColor)); //set initial Line position
  CrossX = ColorPickerX + int(saturation(activeColor)); //set initial Line position
  CrossY = ColorPickerY + int(brightness(activeColor)); //set initial Line position

  String COMx, COMlist = "";
  /*
  Other setup code goes here - I put this at
   the end because of the try/catch structure.
   */
  try {
    if (debug) printArray(Serial.list());
    int i = Serial.list().length;
    if (i != 0) {
      if (i >= 2) {
        // need to check which port the inst uses -
        // for now we'll just let the user decide
        for (int j = 0; j < i; ) {
          COMlist += char(j+'a') + " = " + Serial.list()[j];
          if (++j < i) COMlist += ",  ";
        }
        COMx = showInputDialog("Which COM port is correct? (a,b,..):\n"+COMlist);
        if (COMx == null) exit();
        if (COMx.isEmpty()) exit();
        i = int(COMx.toLowerCase().charAt(0) - 'a') + 1;
      }
      String portName = Serial.list()[i-1];
      if (debug) println(portName);
      myPort = new Serial(this, portName, baud); // change baud rate to your liking
      myPort.bufferUntil('\n'); // buffer until CR/LF appears, but not required..
    } else {
      showMessageDialog(frame, "Device is not connected to the PC");
      exit();
    }
  }
  catch (Exception e)
  { //Print the type of error
    showMessageDialog(frame, "COM port is not available (may\nbe in use by another program)");
    println("Error:", e);
    exit();
  }
}

int red = 0;
int green = 0;
int blue = 0;

int activePresetNum = 0;
int old_activePreset = 0;
void draw()

{

  background(240);
  //drawColorSelector(); 
  drawColorPicker();
  drawLine();
  drawCross();
  drawActiveColor();
  drawValues();
  drawOK();

  red = int(red(activeColor));
  green = int(green(activeColor));
  blue = int(blue(activeColor));



  if (old_red != red || old_green != green || old_blue != blue) {
    message = (str(red(activeColor)) + "," + str(green(activeColor)) + "," + str(blue(activeColor)) + "\0"); //compose message to set coor
    myPort.write(message);
    delay(75);
    old_red = red;
    old_green = green;
    old_blue = blue;
  }
  
  if(old_activePreset != activePresetNum) //We made a selection so write it to the arduino
  {
      message = (presets[activePresetNum] + " \0"); //compose message to set coor
      myPort.write(message);
      println(message);
      delay(100); //Debounce
      
  }
  
  old_activePreset = activePresetNum;

  checkMouse();


  activeColor = color( LineY - ColorPickerY, CrossX - ColorPickerX, 255 - ( CrossY - ColorPickerY ) ); //set current active color
}


void drawColorSelector() 

{

  stroke( interfaceColor );
  strokeWeight( 1 );
  fill( 0 );
  rect( ColorSelectorX, ColorSelectorY, 20, 20 ); //draw color selector border at its x y position

  stroke( 0 );

  if (mouseX>ColorSelectorX&&mouseX<ColorSelectorX+20&&mouseY>ColorSelectorY&&mouseY<ColorSelectorY+20)
    fill( hue(activeColor), saturation(activeColor), brightness(activeColor)+30 );
  else
    fill( activeColor );

  rect( ColorSelectorX + 1, ColorSelectorY + 1, 18, 18 ); //draw the color selector fill 1px inside the border
}

String message;

void drawOK() 
{
  if ( mouseX > ColorPickerX + 285 && mouseX < ColorPickerX + 305 && mouseY > ColorPickerY + 240 - 20 && mouseY < ColorPickerY + 260 -20) { //check if the cross is on the darker color
    fill(0); //optimize visibility on ligher colors
    if (mousePressed == true) {
      println("OFF");
      message = ("0,0,0 \0"); //compose message to set coor
      myPort.write(message);
      delay(100); //Debounce
    }
  } else {
    fill(100); //optimize visibility on darker colors
  }

  text( "OFF", ColorPickerX + 285, ColorPickerY + 250 - 20);

  if ( mouseX > ColorPickerX + 285 && mouseX < ColorPickerX + 305 && mouseY > ColorPickerY + 240 && mouseY < ColorPickerY + 260 ) { //check if the cross is on the darker color
    fill(0); //optimize visibility on ligher colors
    if (mousePressed == true) {
      println("On");
      message = (str(red(activeColor)) + "," + str(green(activeColor)) + "," + str(blue(activeColor)) + "\0"); //compose message to set coor
      myPort.write(message);
      delay(100); //Debounce
    }
  } else {
    fill(100); //optimize visibility on darker colors
  }

  text( "ON", ColorPickerX + 285, ColorPickerY + 250 );
}


void drawValues() 
{
  fill( 255 );
  fill( 0 );
  textSize( 10 );

  text( "H: " + int( ( LineY - ColorPickerY ) * 1.417647 ) + "°", ColorPickerX + 285, ColorPickerY + 100 );
  text( "S: " + int( ( CrossX - ColorPickerX ) * 0.39215 + 0.5 ) + "%", ColorPickerX + 286, ColorPickerY + 115 );
  text( "B: " + int( 100 - ( ( CrossY - ColorPickerY ) * 0.39215 ) ) + "%", ColorPickerX + 285, ColorPickerY + 130 );

  text( "R: " + int( red( activeColor ) ), ColorPickerX + 285, ColorPickerY + 155 );
  text( "G: " + int( green( activeColor ) ), ColorPickerX + 285, ColorPickerY + 170 );
  text( "B: " + int( blue( activeColor ) ), ColorPickerX + 285, ColorPickerY + 185 );

  text( hex( activeColor, 6 ), ColorPickerX + 285, ColorPickerY + 210 );
}

void drawCross() 

{
  if ( brightness( activeColor ) < 90 )
    stroke( 255 );
  else
    stroke( 0 );
  line( CrossX - 5, CrossY, CrossX + 5, CrossY );
  line( CrossX, CrossY - 5, CrossX, CrossY + 5 );
}


void drawLine() 
{
  stroke(0);
  line( ColorPickerX + 259, LineY, ColorPickerX + 276, LineY );
}

void drawColorPicker() 
{
  stroke( interfaceColor );
  //line(ColorSelectorX + 10, ColorSelectorY + 10, ColorPickerX - 3, ColorPickerY - 3 );
  strokeWeight( 1 );
  fill( 0 );
  rect( ColorPickerX - 3, ColorPickerY - 3, 283, 260 );

  loadPixels();
  for ( int j = 0; j < 255; j++ ) //draw a row of pixel with the same brightness but progressive saturation
  {
    for ( int i = 0; i < 255; i++ ) //draw a column of pixel with the same saturation but progressive brightness
      set( ColorPickerX + j, ColorPickerY + i, color( LineY - ColorPickerY, j, 255 - i ) );
  }
  for ( int j = 0; j < 255; j++ )
  {
    for ( int i = 0; i < 20; i++ )
      set( ColorPickerX + 258 + i, ColorPickerY + j, color( j, 255, 255 ) );
  }

  fill( interfaceColor );
  noStroke();
  rect( ColorPickerX + 280, ColorPickerY - 3, 45, 261 );
}

void drawActiveColor() 
{
  fill( activeColor );
  stroke( 0 );
  strokeWeight( 1 );
  rect( ColorPickerX + 282, ColorPickerY - 1, 41, 80 );
}


void checkMouse() 
{
  if ( mousePressed ) 
  {
    if (mouseX>ColorPickerX+258&&mouseX<ColorPickerX+277&&mouseY>ColorPickerY-1&&mouseY<ColorPickerY+255&&!isDraggingCross)
    {
      LineY=mouseY;
      isDraggingLine = true;
    }
    if (mouseX>ColorPickerX-1&&mouseX<ColorPickerX+255&&mouseY>ColorPickerY-1&&mouseY<ColorPickerY+255&&!isDraggingLine)
    {

      CrossX=mouseX;
      CrossY=mouseY;
      isDraggingCross = true;
    }

    if (mouseX>ColorSelectorX&&mouseX<ColorSelectorX+20&&mouseY>ColorSelectorY&&mouseY<ColorSelectorY+20)
      ShowColorPicker = true;

    if (mouseX>ColorPickerX+285&&mouseX<ColorPickerX+305&&mouseY>ColorPickerY+240&&mouseY<ColorPickerY+260)
      ShowColorPicker = false;
  } else

  {
    isDraggingCross = false;
    isDraggingLine = false;
  }
}

void dropdown(int n) {
  /* request the selected item based on index n */
  println(n, cp5.get(ScrollableList.class, "dropdown").getItem(n));

  /* here an item is stored as a Map  with the following key-value pairs:
   * name, the given name of the item
   * text, the given text of the item by default the same as name
   * value, the given value of the item, can be changed by using .getItem(n).put("value", "abc"); a value here is of type Object therefore can be anything
   * color, the given color of the item, how to change, see below
   * view, a customizable view, is of type CDrawable 
   */

  CColor c = new CColor();
  c.setBackground(color(255, 0, 0));
  cp5.get(ScrollableList.class, "dropdown").getItem(n).put("color", c);
}

void controlEvent(ControlEvent theEvent) {
  // DropdownList is of type ControlGroup.
  // A controlEvent will be triggered from inside the ControlGroup class.
  // therefore you need to check the originator of the Event with
  // if (theEvent.isGroup())
  // to avoid an error message thrown by controlP5.
  if (debug == true) {
    if (theEvent.isGroup()) {
      // check if the Event was triggered from a ControlGroup
      println("event from group : "+theEvent.getGroup().getValue()+" from "+theEvent.getGroup());

    } else if (theEvent.isController()) {
      println("event from controller : "+theEvent.getController().getValue()+" from "+theEvent.getController());
            activePresetNum = (int)theEvent.getController().getValue();
    }
  }
}