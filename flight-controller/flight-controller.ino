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
static const unsigned long SENSOR_READ_DELTA_MICROSECONDS = 1000;
static const int pinBluetoothTx = 2;
static const int pinBluetoothRx = 3;
SoftwareSerial bluetooth(pinBluetoothTx, pinBluetoothRx);
HMC5883L compass;
MPU6050 motion;
BMP085 barothermometer;
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
unsigned long lastSensorReadMicroseconds = 0;
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
//  divided by the maximum possible value (250 radians/second)
float scaleGyro  = 1.f / (0x7FFF / 250.f);
// properly scaled v3Accel values, where a magnitude of 1 == 9.81 m/s^2 (1g)
v3f gForce;
// properly scaled v3Gyro values
v3f radiansPerSecond;
v3f relativeOrientationRadians = {0,0,0};
static const size_t NUM_CALIBRATION_ITERATIONS = 30;
v3f calibrationOffsetGyro = {0,0,0};
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
//  motion.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
  //we're changing the gyro's range in the setup function...
//  scaleGyro  = 1.f / (0x7FFF / 1000.f);
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
  // sensor 'calibration': read a bunch of data from the sensors so
  //  we can offset by the average of this data during runtime //
  Serial.println("Calibrating...");
  for(size_t c = 0; c < NUM_CALIBRATION_ITERATIONS; c++)
  {
    motion.getMotion6(&v3Accel.x, &v3Accel.y, &v3Accel.z,
                      &v3Gyro.x , &v3Gyro.y , &v3Gyro.z);
    calibrationOffsetGyro.x += v3Gyro.x;
    calibrationOffsetGyro.y += v3Gyro.y;
    calibrationOffsetGyro.z += v3Gyro.z;
    delay(SENSOR_READ_DELTA_MICROSECONDS / 1000);
  }
  calibrationOffsetGyro.x /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetGyro.y /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetGyro.z /= NUM_CALIBRATION_ITERATIONS;
  Serial.print(calibrationOffsetGyro.x); Serial.print("\t");
  Serial.print(calibrationOffsetGyro.y); Serial.print("\t");
  Serial.print(calibrationOffsetGyro.z); Serial.print("\n");
  Serial.println("---Calibration complete!---");
}
void updateRelativeOrientation(float deltaSeconds)
{
  relativeOrientationRadians.x += radiansPerSecond.x*deltaSeconds;
  relativeOrientationRadians.y += radiansPerSecond.y*deltaSeconds;
  relativeOrientationRadians.z += radiansPerSecond.z*deltaSeconds;
  /*
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
  */
}
void scaleRawMotionData()
{
  // thanks to sasebot-sensei: https://electronics.stackexchange.com/a/176705
  gForce.x = v3Accel.x * scaleAccel;
  gForce.y = v3Accel.y * scaleAccel;
  gForce.z = v3Accel.z * scaleAccel;
  radiansPerSecond.x = (v3Gyro.x - calibrationOffsetGyro.x) * scaleGyro;
  radiansPerSecond.y = (v3Gyro.y - calibrationOffsetGyro.y) * scaleGyro;
  radiansPerSecond.z = (v3Gyro.z - calibrationOffsetGyro.z) * scaleGyro;
}
void loop() 
{
  const unsigned long deltaMicroseconds = 
    micros() - lastSensorReadMicroseconds;
  if(deltaMicroseconds > SENSOR_READ_DELTA_MICROSECONDS)
  {
    motion.getMotion6(&v3Accel.x, &v3Accel.y, &v3Accel.z,
                      &v3Gyro.x , &v3Gyro.y , &v3Gyro.z);
    scaleRawMotionData();
    //compass.getHeading(&v3Compass.x, &v3Compass.y, &v3Compass.z);
    updateRelativeOrientation(deltaMicroseconds / 1000000.f);
    bluetooth.write("FCTP");
    unsigned long const currMicros = micros();
    bluetooth.write((uint8_t const*)&currMicros, sizeof(currMicros));
    bluetooth.write((uint8_t const*)&deltaMicroseconds, sizeof(deltaMicroseconds));
    bluetooth.write((uint8_t const*)&gForce, sizeof(gForce));
    bluetooth.write((uint8_t const*)&radiansPerSecond, 
                    sizeof(radiansPerSecond));
//    bluetooth.write((uint8_t const*)&relativeOrientationRadians, 
//                    sizeof(relativeOrientationRadians));
    bluetooth.write((uint8_t const*)&relativeOrientationRadians, 
                    sizeof(relativeOrientationRadians));
    lastSensorReadMicroseconds = micros();
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
