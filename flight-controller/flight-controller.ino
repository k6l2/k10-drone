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
static const unsigned long SENSOR_READ_DELTA_MILLISECONDS = 50;
static const int pinBluetoothTx = 2;
static const int pinBluetoothRx = 3;
SoftwareSerial bluetooth(pinBluetoothTx, pinBluetoothRx);
HMC5883L compass;
MPU6050 motion;
BMP085 barothermometer;
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
struct i16v3
{
  int16_t x;
  int16_t y;
  int16_t z;
};
i16v3 v3Compass;
// values need to be decoded using the proper sensitivity scaling factors
i16v3 v3Accel;
// values need to be decoded using the proper sensitivity scaling factors
i16v3 v3Gyro;
//float celsius;
//float pascals;
unsigned long lastSensorReadMilliseconds = 0;
struct v3f
{
  float x;
  float y;
  float z;
};
// see MPU6050::initialize for why this variable is initialized to this value
//  The sensor's register returns a signed-16-bit #, and that range must be
//  divided by the maximum possible value (2G)
float scaleAccel = 1.f / (0x7FFF / 2.f);
// see MPU6050::initialize for why this variable is initialized to this value
//  The sensor's register returns a signed-16-bit #, and that range must be
//  divided by the maximum possible value (250 degrees/second)
float scaleGyro  = 1.f / (0x7FFF / 250.f);
// properly scaled v3Accel values, where a magnitude of 1 == 9.81 m/s^2 (1g)
v3f gForce;
// properly scaled v3Gyro values
v3f degreesPerSecond;
v3f relativeOrientationRadians = {0,0,0};
static const float REL_ORIENT_UPDATE_BIAS = 0.98;
void updateRelativeOrientation(float deltaSeconds)
{
  // Derived from: https://www.w3.org/TR/motion-sensors/#complementary-filters
  static const float ACCEL_SCALE = PI / 2;
  const float gForceMag = sqrtf(gForce.x*gForce.x + 
                                gForce.y*gForce.y + 
                                gForce.z*gForce.z);
  if(gForceMag > 0)
  {
    relativeOrientationRadians.x = 
           REL_ORIENT_UPDATE_BIAS *(relativeOrientationRadians.x + degreesPerSecond.x*(PI/180)*deltaSeconds) +
      (1 - REL_ORIENT_UPDATE_BIAS)*(gForce.x / gForceMag *  ACCEL_SCALE);
    relativeOrientationRadians.y = 
           REL_ORIENT_UPDATE_BIAS *(relativeOrientationRadians.y + degreesPerSecond.y*(PI/180)*deltaSeconds) +
      (1 - REL_ORIENT_UPDATE_BIAS)*(gForce.y / gForceMag * -ACCEL_SCALE);
  }
  relativeOrientationRadians.z += degreesPerSecond.z*(PI/180)*deltaSeconds;
}
void scaleRawMotionData()
{
  // thanks to sasebot-sensei: https://electronics.stackexchange.com/a/176705
  gForce.x = v3Accel.x * scaleAccel;
  gForce.y = v3Accel.y * scaleAccel;
  gForce.z = v3Accel.z * scaleAccel;
  degreesPerSecond.x = v3Gyro.x * scaleGyro;
  degreesPerSecond.y = v3Gyro.y * scaleGyro;
  degreesPerSecond.z = v3Gyro.z * scaleGyro;
}
void loop() 
{
  const unsigned long deltaMilliseconds = 
    millis() - lastSensorReadMilliseconds;
  if(deltaMilliseconds > SENSOR_READ_DELTA_MILLISECONDS)
  {
    motion.getMotion6(&v3Accel.x, &v3Accel.y, &v3Accel.z,
                      &v3Gyro.x , &v3Gyro.y , &v3Gyro.z);
    scaleRawMotionData();
    compass.getHeading(&v3Compass.x, &v3Compass.y, &v3Compass.z);
    updateRelativeOrientation(deltaMilliseconds / 1000.f);
    /*
    Serial.print(deltaMilliseconds); Serial.print("\t");
    Serial.print(relativeOrientationRadians.x); Serial.print("\t");
    Serial.print(relativeOrientationRadians.y); Serial.print("\t");
    Serial.print(relativeOrientationRadians.z); Serial.print("\n");
//    const float heading = atan2(v3Compass.y, v3Compass.x);
    Serial.print(gForce.x);           Serial.print("\t");
    Serial.print(gForce.y);           Serial.print("\t");
    Serial.print(gForce.z);           Serial.print("\t");
    Serial.print(degreesPerSecond.x); Serial.print("\t");
    Serial.print(degreesPerSecond.y); Serial.print("\t");
    Serial.print(degreesPerSecond.z); Serial.print("\n");
    Serial.print(v3Accel.x);        Serial.print("\t");
    Serial.print(v3Accel.y);        Serial.print("\t");
    Serial.print(v3Accel.z);        Serial.print("\t");
    Serial.print(v3Gyro.x);         Serial.print("\t");
    Serial.print(v3Gyro.y);         Serial.print("\t");
    Serial.print(v3Gyro.z);         Serial.print("\n");
    Serial.print(v3Compass.x);      Serial.print("\t");
    Serial.print(v3Compass.y);      Serial.print("\t");
    Serial.print(v3Compass.z);      Serial.print("\t");
    Serial.print(heading*180/M_PI); Serial.print("\t\n");*/
    bluetooth.write("FCTP");
    unsigned long const currMillis = millis();
    bluetooth.write((uint8_t const*)&currMillis, sizeof(unsigned long));
    bluetooth.write((uint8_t const*)&relativeOrientationRadians, 
                    sizeof(relativeOrientationRadians));
//    bluetooth.write((uint8_t const*)&v3Accel   , sizeof(i16v3));
//    bluetooth.write((uint8_t const*)&v3Gyro    , sizeof(i16v3));
//    bluetooth.write((uint8_t const*)&v3Compass , sizeof(i16v3));
    /*bluetooth.print(v3Accel.x);        bluetooth.print("\t");
    bluetooth.print(v3Accel.y);        bluetooth.print("\t");
    bluetooth.print(v3Accel.z);        bluetooth.print("\t");
    bluetooth.print(v3Gyro.x);         bluetooth.print("\t");
    bluetooth.print(v3Gyro.y);         bluetooth.print("\t");
    bluetooth.print(v3Gyro.z);         bluetooth.print("\t");
    bluetooth.print(v3Compass.x);      bluetooth.print("\t");
    bluetooth.print(v3Compass.y);      bluetooth.print("\t");
    bluetooth.print(v3Compass.z);      bluetooth.print("\t");
    bluetooth.print(heading*180/M_PI); bluetooth.print("\t\n");*/
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
