/*
 * Hardware required:
 *  -Arduino Uno
 *  -Sparkfun Bluesmirf Silver
 *    https://www.sparkfun.com/products/12577
 *  -GY-87
 *    http://kom.aau.dk/~jdn/edu/doc/sensors/gy80gy87gy88/
 * This sketch will require the GY-87 board use the following connections
 *  on the Arduino Uno:
 *    SDA => A4
 *    SCL => A5
 * I am going to take a lot of code from this repo:
 * https://github.com/guywithaview/Arduino-Test/blob/master/GY87/GY87.ino
 * Another sketch using the GY-87 can be found here:
 * http://www.techmonkeybusiness.com/the-gy87-combined-sensor-test-sketch.html
 *    But this seems out-of-date and probably isn't too useful.
 * Using I2CDevLib found here:
 * https://github.com/jrowberg/i2cdevlib
 * rudimentary info on the Wire Library can be found here:
 * https://www.arduino.cc/en/reference/wire
 */
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MPU6050.h>
#include <HMC5883L.h>
#include <BMP085.h>
#include <I2Cdev.h>
static const unsigned long SENSOR_READ_DELTA_MILLISECONDS = 100;
static const int pinBluetoothTx = 2;
static const int pinBluetoothRx = 3;
static const float SENSITIVITY_ACCEL = 16384;
static const float SENSITIVITY_GYRO  = 131;
struct i16v3
{
  int16_t x;
  int16_t y;
  int16_t z;
};
SoftwareSerial bluetooth(pinBluetoothTx, pinBluetoothRx);
HMC5883L compass;
i16v3 v3Compass;
MPU6050 motion;
i16v3 v3Accel;
i16v3 v3Gyro;
BMP085 barothermometer;
//float celsius;
//float pascals;
unsigned long lastSensorReadMilliseconds = 0;
void setup()
{
  // Start the atmega's serial monitor so we can examine the COM port output
  Serial.begin(9600);
  Serial.println("---Drone Flight Controller---");
  // the bluesmirf starts w/ a baud of 115200
  bluetooth.begin(115200);
  // enter command mode
  bluetooth.print("$$$");
  // wait for the modem to send back "CMD"
  delay(100);
  // U command changes the UART and immediately exits command mode
  //  first parameter is a 4-character baud rate
  //  second parameter is parity (E, O or N)
  bluetooth.println("U,9600,N");
  // switch the bluetooth serial to 9600 baud.  We do this because apparently
  //  115200 baud can be too fast for NewSoftSerial to relay data reliably
  bluetooth.begin(9600);
  // Initialize the I2C bus
  Wire.begin();
  // Initialize the MPU6050 chip on the GY-87
  motion.initialize();
  Serial.print("Accellerometer/Gyro connection... ");
  Serial.println(motion.testConnection() ? "Success!" : "FAILURE!!!");
  // in order to use the HMC5883L, we need to enable an I2C bypass
  motion.setI2CBypassEnabled(true);
  // initialize the HMC5883L chip on the GY-87
  compass.initialize();
  Serial.print("Compass connection............... ");
  Serial.println(motion.testConnection() ? "Success!" : "FAILURE!!!");
  // initialize the BMP085 chip on the GY-87
  barothermometer.initialize();
  Serial.print("Barothermometer connection....... ");
  Serial.println(motion.testConnection() ? "Success!" : "FAILURE!!!");
  Serial.println("---Setup complete!---");
}
void loop() 
{
  if(millis() - lastSensorReadMilliseconds > SENSOR_READ_DELTA_MILLISECONDS)
  {
    motion.getMotion6(&v3Accel.x, &v3Accel.y, &v3Accel.z,
                      &v3Gyro.x, &v3Gyro.y, &v3Gyro.z);
    compass.getHeading(&v3Compass.x, &v3Compass.y, &v3Compass.z);
    const float heading = atan2(v3Compass.y, v3Compass.x);
    /*Serial.print(v3Accel.x);        Serial.print("\t");
    Serial.print(v3Accel.y);        Serial.print("\t");
    Serial.print(v3Accel.z);        Serial.print("\t");
    Serial.print(v3Gyro.x);         Serial.print("\t");
    Serial.print(v3Gyro.y);         Serial.print("\t");
    Serial.print(v3Gyro.z);         Serial.print("\t");
    Serial.print(v3Compass.x);      Serial.print("\t");
    Serial.print(v3Compass.y);      Serial.print("\t");
    Serial.print(v3Compass.z);      Serial.print("\t");
    Serial.print(heading*180/M_PI); Serial.print("\t\n");*/
    bluetooth.print(v3Accel.x);        bluetooth.print("\t");
    bluetooth.print(v3Accel.y);        bluetooth.print("\t");
    bluetooth.print(v3Accel.z);        bluetooth.print("\t");
    bluetooth.print(v3Gyro.x);         bluetooth.print("\t");
    bluetooth.print(v3Gyro.y);         bluetooth.print("\t");
    bluetooth.print(v3Gyro.z);         bluetooth.print("\t");
    bluetooth.print(v3Compass.x);      bluetooth.print("\t");
    bluetooth.print(v3Compass.y);      bluetooth.print("\t");
    bluetooth.print(v3Compass.z);      bluetooth.print("\t");
    bluetooth.print(heading*180/M_PI); bluetooth.print("\t\n");
    lastSensorReadMilliseconds = millis();
  }
  if(bluetooth.available())
  {
    Serial.print((char)bluetooth.read());
  }
  if(Serial.available())
  {
    bluetooth.print((char)Serial.read());
  }
}
