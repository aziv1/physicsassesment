#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "MPU6050.h" // Use the MPU6050 library

MPU6050 mpu;

// SPI pins for SD card
int miso = 2;
int mosi = 3;
int sck = 4;
int cs = 10;

// I2C pins for MPU6050
int sda = 8;
int scl = 9;

// Sampling rate in milliseconds (e.g., 25ms = 40Hz)
const unsigned long sampleInterval = 25;
unsigned long lastSampleTime = 0;

File dataFile;

// MPU6050 Register Addresses
const int MPU6050_ADDR = 0x68; // Default I2C address
const int PWR_MGMT_1   = 0x6B;
const int ACCEL_XOUT_H = 0x3B;
const int ACCEL_YOUT_H = 0x3D;
const int ACCEL_ZOUT_H = 0x3F;
const int GYRO_XOUT_H  = 0x43;
const int GYRO_YOUT_H  = 0x45;
const int GYRO_ZOUT_H  = 0x47;
const int ACCEL_CONFIG = 0x1C;
const int GYRO_CONFIG  = 0x1B;

// Calibration Data Structure
struct CalibrationData {
  float accelXBias;
  float accelYBias;
  float accelZBias;
  float gyroXBias;
  float gyroYBias;
  float gyroZBias;
};

// Global variables for the selected ranges
int accelRangeSetting = 3; // 0: +/-2g, 1: +/-4g, 2: +/-8g, 3: +/-16g
int gyroRangeSetting = 1; // 0: +/-250dps, 1: +/-500dps, 2: +/-1000dps, 3: +/-2000dps

// Conversion factors for calculating g and dps values
float accel_scale;
float gyro_scale;
CalibrationData calibrationData; // Global calibration data

// Function Declarations
void initializeMPU6050();
CalibrationData calibrateMPU6050(int numSamples);
void readAndProcessData(const CalibrationData& calibration);
int16_t readRawData(int address);
void setFullScaleRange(int accelRange, int gyroRange);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // I2C for MPU6050
  Wire.begin(sda, scl);
  mpu.initialize();

  // Select and set the desired ranges
  accelRangeSetting = 3; // Example: +/-16g
  gyroRangeSetting = 1; // Example: +/-500dps
  setFullScaleRange(accelRangeSetting, gyroRangeSetting);

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not connected!");
    while (1);
  }

  // Calibrate the MPU6050
  calibrationData = calibrateMPU6050(1000); // Calibrate at startup

  // SPI for SD card
  SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {
    Serial.println("SD card mount failed!");
    while (1);
  }

  // Create/open file
  dataFile = SD.open("/imu_log.csv", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Failed to open file");
    while (1);
  }

  // Write headers
  dataFile.println("Time(ms),Ax(m/s^2),Ay(m/s^2),Az(m/s^2),Gx(deg/s),Gy(deg/s),Gz(deg/s)");
  dataFile.close();

  Serial.println("Logging started.");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime >= sampleInterval) {
    lastSampleTime = now;
    readAndProcessData(calibrationData); // Use the global calibration data
  }
}

void initializeMPU6050() {
  Wire.begin(sda, scl); // Use the specified SDA and SCL pins
  mpu.initialize();
  setFullScaleRange(accelRangeSetting, gyroRangeSetting); // Set the full scale ranges
  delay(100);
}

void setFullScaleRange(int accelRange, int gyroRange) {
  // Set accelerometer range
  mpu.setFullScaleAccelRange(accelRange);
  Serial.print("Accelerometer range set to ");
  switch (accelRange) {
    case 0:
      Serial.println("+/- 2g");
      accel_scale = 16384.0;
      break;
    case 1:
      Serial.println("+/- 4g");
      accel_scale = 8192.0;
      break;
    case 2:
      Serial.println("+/- 8g");
      accel_scale = 4096.0;
      break;
    case 3:
      Serial.println("+/- 16g");
      accel_scale = 2048.0;
      break;
  }

  // Set gyroscope range
  mpu.setFullScaleGyroRange(gyroRange);
  Serial.print("Gyroscope range set to ");
  switch (gyroRange) {
    case 0:
      Serial.println("+/- 250 dps");
      gyro_scale = 131.0;
      break;
    case 1:
      Serial.println("+/- 500 dps");
      gyro_scale = 65.5;
      break;
    case 2:
      Serial.println("+/- 1000 dps");
      gyro_scale = 32.8;
      break;
    case 3:
      Serial.println("+/- 2000 dps");
      gyro_scale = 16.4;
      break;
  }
}

CalibrationData calibrateMPU6050(int numSamples) {
  long accelXSum = 0, accelYSum = 0, accelZSum = 0;
  long gyroXSum = 0, gyroYSum = 0, gyroZSum = 0;

  Serial.println("Starting calibration... Keep the sensor still.");
  delay(2000); // Give time to stabilize

  for (int i = 0; i < numSamples; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    accelXSum += ax;
    accelYSum += ay;
    accelZSum += az;
    gyroXSum += gx;
    gyroYSum += gy;
    gyroZSum += gz;
    delay(2);
  }

  CalibrationData calibration;
  calibration.accelXBias = (float)accelXSum / numSamples;
  calibration.accelYBias = (float)accelYSum / numSamples;
  calibration.accelZBias = (float)accelZSum / numSamples;
  calibration.gyroXBias = (float)gyroXSum / numSamples;
  calibration.gyroYBias = (float)gyroYSum / numSamples;
  calibration.gyroZBias = (float)gyroZSum / numSamples;

  Serial.println("Calibration complete.");
  Serial.print("Accel Bias (X, Y, Z): ");
  Serial.print(calibration.accelXBias); Serial.print(", ");
  Serial.print(calibration.accelYBias); Serial.print(", ");
  Serial.println(calibration.accelZBias);
  Serial.print("Gyro Bias (X, Y, Z): ");
  Serial.print(calibration.gyroXBias); Serial.print(", ");
  Serial.print(calibration.gyroYBias); Serial.print(", ");
  Serial.println(calibration.gyroZBias);

  return calibration;
}

void readAndProcessData(const CalibrationData& calibration) {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Apply calibration
  float accelXCalibrated = ax - calibration.accelXBias;
  float accelYCalibrated = ay - calibration.accelYBias;
  float accelZCalibrated = az - calibration.accelZBias;
  float gyroXCalibrated = gx - calibration.gyroXBias;
  float gyroYCalibrated = gy - calibration.gyroYBias;
  float gyroZCalibrated = gz - calibration.gyroZBias;

  // Convert to physical units using the pre-calculated scaling factors
  float ax_m2 = (accelXCalibrated / accel_scale) * 9.80665;
  float ay_m2 = (accelYCalibrated / accel_scale) * 9.80665;
  float az_m2 = (accelZCalibrated / accel_scale) * 9.80665;
  float gx_dps = gyroXCalibrated / gyro_scale;
  float gy_dps = gyroYCalibrated / gyro_scale;
  float gz_dps = gyroZCalibrated / gyro_scale;

  unsigned long now = millis();
  // Serial print
  Serial.printf("T=%lu ms | Ax=%.2f Ay=%.2f Az=%.2f m/s² | Gx=%.2f Gy=%.2f Gz=%.2f deg/s\n",
                now, ax_m2, ay_m2, az_m2, gx_dps, gy_dps, gz_dps);

  // Write to SD card
  dataFile = SD.open("/imu_log.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%lu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                  now, ax_m2, ay_m2, az_m2, gx_dps, gy_dps, gz_dps);
    dataFile.close();
  } else {
    Serial.println("Error writing to file!");
  }
}

int16_t readRawData(int address) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 2, true);
  return (Wire.read() << 8) | Wire.read();
}
