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
float scaleAccel = 1.f / (0x7FFF / 2.f/*G*/);
// see MPU6050::initialize for why this variable is initialized to this value
//  The sensor's register returns a signed-16-bit #, and that range must be
//  divided by the maximum possible value (250 radians/second)
float scaleGyro  = 1.f / (0x7FFF / 250.f/*radians/second*/);
// properly scaled v3Accel values, where a magnitude of 1 == 9.81 m/s^2 (1g)
v3f gForce;
// properly scaled v3Gyro values
v3f radiansPerSecond;
v3f relativeOrientationRadians = {0,0,0};
static const size_t NUM_CALIBRATION_ITERATIONS = 30;
v3f calibrationOffsetGyro  = {0,0,0};
v3f calibrationOffsetAccel = {0,0,0};
// gForceMedianFilterBuffer should be used as a circular buffer
static const size_t G_FORCE_MEDIAN_FILTER_BUFFER_SIZE = 10;
v3f gForceMedianFilterBuffer[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE];
size_t gForceMedianFilterBufferOffset = 0;
v3f gForceMedian = {0,0,0};
void scaleRawMotionData()
{
  // thanks to sasebot-sensei: https://electronics.stackexchange.com/a/176705
  // Also, offset raw values by calibrated amounts calculated during setup
  gForce.x = (v3Accel.x - calibrationOffsetAccel.x) * scaleAccel;
  gForce.y = (v3Accel.y - calibrationOffsetAccel.y) * scaleAccel;
  gForce.z = (v3Accel.z - calibrationOffsetAccel.z) * scaleAccel;
  radiansPerSecond.x = (v3Gyro.x - calibrationOffsetGyro.x) * scaleGyro;
  radiansPerSecond.y = (v3Gyro.y - calibrationOffsetGyro.y) * scaleGyro;
  radiansPerSecond.z = (v3Gyro.z - calibrationOffsetGyro.z) * scaleGyro;
}
void setup()
{
  // Start the atmega's serial monitor so we can examine the COM port output
  Serial.begin(9600);
  Serial.println("---Drone Flight Controller---");
  Serial.println("Configuring bluetooth modem to 9600 baud...");
  bluetooth.begin(115200);// the bluesmirf starts w/ a baud of 115200
  bluetooth.print("$$$");// enter command mode
  delay(100);// wait for the modem to send back "CMD"
  bluetooth.println("U,9600,N");// U command changes the UART and immediately exits command mode
                                //  first parameter is a 4-character baud rate
                                //  second parameter is parity (E, O or N)
  bluetooth.begin(9600);// switch the bluetooth serial to 9600 baud.  We do this because apparently
                        //  115200 baud can be too fast for NewSoftSerial to relay data reliably
  Serial.println("Reprogramming bluetooth modem credentials...");
  bluetooth.print("$$$");// enter command mode
  delay(100);// wait for the modem to send back "CMD"
  bluetooth.println("S-,""K6L2-QuadFcData");// set the bluetooth device name, auto-appending the last 2 bytes of the modem's MAC address
  delay(100);// wait for the modem to send back "AOK"
  bluetooth.println("SP,""6942");// set the bluetooth PIN
  delay(100);// wait for the modem to send back "AOK"
  bluetooth.println("---");// leave command mode
  delay(100);// wait for the modem to send back "END"
  Serial.println("---Bluetooth Modem Configured---");
  Wire.begin();// Initialize the I2C bus
  motion.initialize();// Initialize the MPU6050 chip on the GY-87
  Serial.print("Accellerometer/Gyro connection... ");
  Serial.println(motion.testConnection() ? "Success!" : "FAILURE!!!");
  motion.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);   // increase the gyro's range (decrease sensitivity)
  scaleGyro = 1.f / (0x7FFF / 1000.f/*radians/second*/);// since we changed the device scale, we have to change our f32 scale factor
  motion.setI2CBypassEnabled(true);// in order to use the HMC5883L, we need to enable an I2C bypass
  compass.initialize();// initialize the HMC5883L chip on the GY-87
  Serial.print("Compass connection............... ");
  Serial.println(compass.testConnection() ? "Success!" : "FAILURE!!!");
  barothermometer.initialize();// initialize the BMP085 chip on the GY-87
  Serial.print("Barothermometer connection....... ");
  Serial.println(barothermometer.testConnection() ? "Success!" : "FAILURE!!!");
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
    calibrationOffsetAccel.x += v3Accel.x;
    calibrationOffsetAccel.y += v3Accel.y;
    // Accelerometer in the Z direction must be calibrated for 1G,
    //  since it is assumed that the drone is flat on the ground
    //  during calibration //
    calibrationOffsetAccel.z += (v3Accel.z - (1/scaleAccel));
    delay(1);
  }
  calibrationOffsetGyro.x /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetGyro.y /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetGyro.z /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetAccel.x /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetAccel.y /= NUM_CALIBRATION_ITERATIONS;
  calibrationOffsetAccel.z /= NUM_CALIBRATION_ITERATIONS;
  Serial.print(calibrationOffsetGyro.x); Serial.print("\t");
  Serial.print(calibrationOffsetGyro.y); Serial.print("\t");
  Serial.print(calibrationOffsetGyro.z); Serial.print("\n");
  Serial.print(calibrationOffsetAccel.x); Serial.print("\t");
  Serial.print(calibrationOffsetAccel.y); Serial.print("\t");
  Serial.print(calibrationOffsetAccel.z); Serial.print("\n");
  // build the initial state of our gForce median filter buffer using
  //  the calibration data we just extracted //
  for(size_t c = 0; c < G_FORCE_MEDIAN_FILTER_BUFFER_SIZE; c++)
  {
    motion.getMotion6(&v3Accel.x, &v3Accel.y, &v3Accel.z,
                      &v3Gyro.x , &v3Gyro.y , &v3Gyro.z);
    scaleRawMotionData();
    gForceMedianFilterBuffer[c] = gForce;
    delay(1);
  }
  Serial.println("---Calibration complete!---");
}
void updateRelativeOrientation(float deltaSeconds)
{
  // naive gyroscope integration //
//  relativeOrientationRadians.x += radiansPerSecond.x*deltaSeconds;
//  relativeOrientationRadians.y += radiansPerSecond.y*deltaSeconds;
//  relativeOrientationRadians.z += radiansPerSecond.z*deltaSeconds;
  const v3f gyroDelta = {
    radiansPerSecond.x*deltaSeconds,
    radiansPerSecond.y*deltaSeconds,
    radiansPerSecond.z*deltaSeconds
  };
  // accelerometer angle change calculations using equations 25 & 26
  //  from this paper: https://www.nxp.com/docs/en/application-note/AN3461.pdf
//  const float accelPhi   = atan2f(gForceMedian.y, gForceMedian.z);
//  const float accelTheta = atan2f(-gForceMedian.x, 
//                                  sqrtf(powf(gForceMedian.y,2) + 
//                                        powf(gForceMedian.z,2)));
//  relativeOrientationRadians.x += accelPhi;
//  relativeOrientationRadians.y += accelTheta;
  // Complementary filter - combine the gyroscope w/ filtered accelerometer //
//  static const float GYRO_PART = 0.995;
//  relativeOrientationRadians.x = 
//    GYRO_PART       * (relativeOrientationRadians.x + gyroDelta.x) +
//    (1 - GYRO_PART) * accelPhi;
//  relativeOrientationRadians.y = 
//    GYRO_PART       * (relativeOrientationRadians.y + gyroDelta.y) +
//    (1 - GYRO_PART) * accelTheta;
  // Derived from: https://www.w3.org/TR/motion-sensors/#complementary-filters
  static const float ACCEL_SCALE = PI / 2;
  static const float GYRO_BIAS = 0.98;
  const float gForceMag = sqrtf(gForce.x*gForce.x + 
                                gForce.y*gForce.y + 
                                gForce.z*gForce.z);
  if(gForceMag > 0)
  {
    relativeOrientationRadians.x =      GYRO_BIAS *(relativeOrientationRadians.x + gyroDelta.x) 
                                 + (1 - GYRO_BIAS)*(gForce.x / gForceMag *  ACCEL_SCALE);
    relativeOrientationRadians.y =      GYRO_BIAS *(relativeOrientationRadians.y + gyroDelta.y) 
                                 + (1 - GYRO_BIAS)*(gForce.y / gForceMag * -ACCEL_SCALE);
  }
  relativeOrientationRadians.z += gyroDelta.z;
}
int sortFloatsDecending(void const* floatA, void const* floatB)
{
  // Thanks, Johnny-sensei: https://arduino.stackexchange.com/a/38179
  float const a = *((float*)floatA);
  float const b = *((float*)floatB);
  return a > b ? -1 : (a < b ? 1 : 0);
}
void applyGForceMedianFilter()
{
  float gForceMedianFilterBufferX[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE];
  float gForceMedianFilterBufferY[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE];
  float gForceMedianFilterBufferZ[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE];
  // Add the next gForce data point to the gForceMedianFilterBuffer and circle
  //  the gForceMedianFilterBufferOffset to the next value
  gForceMedianFilterBuffer[gForceMedianFilterBufferOffset] = gForce;
  gForceMedianFilterBufferOffset++;
  gForceMedianFilterBufferOffset %= G_FORCE_MEDIAN_FILTER_BUFFER_SIZE;
  // Copy the median filter buffer to local storage
  for(size_t c = 0; c < G_FORCE_MEDIAN_FILTER_BUFFER_SIZE; c++)
  {
    gForceMedianFilterBufferX[c] = gForceMedianFilterBuffer[c].x;
    gForceMedianFilterBufferY[c] = gForceMedianFilterBuffer[c].y;
    gForceMedianFilterBufferZ[c] = gForceMedianFilterBuffer[c].z;
  }
  // Sort the local copy of the buffer
  qsort(gForceMedianFilterBufferX, G_FORCE_MEDIAN_FILTER_BUFFER_SIZE,
        sizeof(gForceMedianFilterBufferX[0]), sortFloatsDecending);
  qsort(gForceMedianFilterBufferY, G_FORCE_MEDIAN_FILTER_BUFFER_SIZE,
        sizeof(gForceMedianFilterBufferY[0]), sortFloatsDecending);
  qsort(gForceMedianFilterBufferZ, G_FORCE_MEDIAN_FILTER_BUFFER_SIZE,
        sizeof(gForceMedianFilterBufferZ[0]), sortFloatsDecending);
  // Write the median gForce value out to some piece of memory
  gForceMedian.x = gForceMedianFilterBufferX[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE/2];
  gForceMedian.y = gForceMedianFilterBufferY[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE/2];
  gForceMedian.z = gForceMedianFilterBufferZ[G_FORCE_MEDIAN_FILTER_BUFFER_SIZE/2];
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
    applyGForceMedianFilter();
    //compass.getHeading(&v3Compass.x, &v3Compass.y, &v3Compass.z);
    updateRelativeOrientation(deltaMicroseconds / 1000000.f);
    bluetooth.write("FCTP");
    unsigned long const currMicros = micros();
    bluetooth.write((uint8_t const*)&currMicros                , sizeof(currMicros));
    bluetooth.write((uint8_t const*)&deltaMicroseconds         , sizeof(deltaMicroseconds));
    bluetooth.write((uint8_t const*)&gForce                    , sizeof(gForce));
    bluetooth.write((uint8_t const*)&gForceMedian              , sizeof(gForceMedian));
    bluetooth.write((uint8_t const*)&radiansPerSecond          , sizeof(radiansPerSecond));
    bluetooth.write((uint8_t const*)&relativeOrientationRadians, sizeof(relativeOrientationRadians));
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
