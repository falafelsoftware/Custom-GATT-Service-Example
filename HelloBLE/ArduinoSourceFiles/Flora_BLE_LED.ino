#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <String.h>

#define BLUEFRUIT_HWSERIAL_NAME      Serial1
#define BLUEFRUIT_UART_MODE_PIN        12    //we aren't using this because we aren't switching modes
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output
#define NEO_PIXEL_PIN                   8    //On-board NeoPixel on Flora 2


Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
String currentColorValue = "FF-FF-FF";
Adafruit_NeoPixel onBoardPixel = Adafruit_NeoPixel(1, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
int redVal = 255;
int greenVal = 255;
int blueVal = 255;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  //Set Red LED
  redVal = 255;
  greenVal = 0;
  blueVal = 0;
  setPixelColor();
  while (1);
}

void setup() {
  //remove these 2 lines if not debugging
  while (!Serial); // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Flora BLE LED Example"));
  Serial.println(F("---------------------------------------------------"));

  randomSeed(micros());

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea
  // anyways for the super long lines ahead!
  ble.setInterCharWriteDelay(5); // 5 ms
  Serial.println();

  onBoardPixel.begin();
  setPixelColor();

}

void loop() {
  bool hasChanged = false;
 
  //check writable characteristic for a new color
  ble.println("AT+GATTCHAR=2");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  // Some data was found, its in the buffer
  Serial.print(F("[Recv] ")); 
  Serial.println(ble.buffer); //format FF-FF-FF R-G-B
  
  String buffer = String(ble.buffer);
  //check if the color should be changed
  if(buffer!=currentColorValue){
    hasChanged = true;
    currentColorValue = buffer;
    String redBuff = buffer.substring(0,2);
    String greenBuff = buffer.substring(3,5);
    String blueBuff = buffer.substring(6); 
    const char* red = redBuff.c_str();
    const char* green =  greenBuff.c_str();
    const char* blue = blueBuff.c_str();
    redVal = strtoul(red,NULL,16);
    greenVal = strtoul(green, NULL, 16);
    blueVal = strtoul(blue,NULL,16);
    setPixelColor();
  }
  ble.waitForOK();
  
  if(hasChanged){
    Serial.println("Notify of color change");
    //write to notifiable characteristic
    String notifyCommand = "AT+GATTCHAR=1,"+ currentColorValue;
    ble.println( notifyCommand ); 
    if ( !ble.waitForOK() )
    {
      error(F("Failed to get response from notify property update"));
    }

    //write to readable characteristic for current color
    String readableCommand = "AT+GATTCHAR=3,"+ currentColorValue;
    ble.println(readableCommand);

    if ( !ble.waitForOK() )
    {
      error(F("Failed to get response from readable property update"));
    }
  }
  delay(1000);
}

void setPixelColor() {
  onBoardPixel.setPixelColor(0,onBoardPixel.Color(redVal, greenVal, blueVal));
  onBoardPixel.show();
}


