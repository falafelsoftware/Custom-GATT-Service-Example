#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"

#define BLUEFRUIT_HWSERIAL_NAME      Serial1
#define BLUEFRUIT_UART_MODE_PIN        12    // we won't be changing modes
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output

// Create the bluefruit object

// Flora uses hardware serial, which does not need the RTS/CTS pins
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// The service and characteristic index information 
int32_t gattServiceId;
int32_t gattNotifiableCharId;
int32_t gattWritableResponseCharId;
int32_t gattWritableNoResponseCharId;
int32_t gattReadableCharId;

void setup(void)
{
  //remove these 2 lines if not debugging - nothing will start until you open the serial window
  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  Serial.println(F("Adafruit Custom GATT Service Example"));
  Serial.println(F("---------------------------------------------------"));

  randomSeed(micros());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea
  // anyways for the super long lines ahead!
  ble.setInterCharWriteDelay(5); // 5 ms


  /* Add the Custom GATT Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Custom GATT Service definition: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID128=00-77-13-12-11-00-00-00-00-00-AB-BA-0F-A1-AF-E1"), &gattServiceId);
  if (! success) {
    error(F("Could not add Custom GATT service"));
  }

  /* Add the Readable/Notifiable characteristic - this characteristic will be set every time the color of the LED is changed */
  /* Characteristic ID should be 1 */
  /* 0x00FF00 == RGB HEX of GREEN */
  Serial.println(F("Adding the Notifiable characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=00-68-42-01-14-88-59-77-42-42-AB-BA-0F-A1-AF-E1,PROPERTIES=0x12,MIN_LEN=1, MAX_LEN=3, VALUE=0x00FF00"), &gattNotifiableCharId);
    if (! success) {
    error(F("Could not add Custom Notifiable characteristic"));
  }
  
  /* Add the Writable characteristic - an external device writes to this characteristic to change the LED color */
  /* Characteristic ID should be 2 */
  Serial.println(F("Adding the Writable with Response characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=00-69-42-03-00-77-12-10-13-42-AB-BA-0F-A1-AF-E1,PROPERTIES=0x08,MIN_LEN=1, MAX_LEN=3, VALUE=0x00FF00"), &gattWritableNoResponseCharId);
    if (! success) {
    error(F("Could not add Custom Writable characteristic"));
  }

  /* Add the Readable characteristic - external devices can query the current LED color using this characteristic */
  /* Characteristic ID should be 3 */
  Serial.println(F("Adding the Readable characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=00-70-42-04-00-77-12-10-13-42-AB-BA-0F-A1-AF-E1,PROPERTIES=0x02,MIN_LEN=1, MAX_LEN=3, VALUE=0x00FF00"), &gattReadableCharId);
    if (! success) {
    error(F("Could not add Custom Readable characteristic"));
  }

  /* Add the Custom GATT Service to the advertising data */
  //0x1312 from AT+GATTLIST - 16 bit svc id
  Serial.print(F("Adding Custom GATT Service UUID to the advertising payload: "));
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-03-02-12-13") );

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  Serial.println();
}

void loop(void)
{

}
