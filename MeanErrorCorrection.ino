#include <Wire.h>

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

// Function Declarations
void initializeMPU6050();
CalibrationData calibrateMPU6050(int numSamples);
void readAndProcessData(const CalibrationData& calibration);
int16_t readRawData(int address);
void setFullScaleRange(int accelRange, int gyroRange);

// Global variables for the selected ranges
int accelRangeSetting = 3; // 0: +/-2g, 1: +/-4g, 2: +/-8g, 3: +/-16g
int gyroRangeSetting  = 1; // 0: +/-250dps, 1: +/-500dps, 2: +/-1000dps, 3: +/-2000dps

// Conversion factors for calculating g and dps values
float accel_scale;
float gyro_scale;

void setup() {
  Serial.begin(115200);

  // Select the desired ranges.  Change these values as needed.
  // 0: +/-2g, 1: +/-4g, 2: +/-8g, 3: +/-16g
  accelRangeSetting = 0;
  // 0: +/-250dps, 1: +/-500dps, 2: +/-1000dps, 3: +/-2000dps
  gyroRangeSetting  = 0;

  initializeMPU6050();
}

void loop() {
  static CalibrationData calibration = calibrateMPU6050(1000); // Calibrate at startup
  readAndProcessData(calibration);
}

// Function Definitions

void initializeMPU6050() {
  Wire.begin();
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0); // Wake up the MPU6050
  Wire.endTransmission(true);
  delay(100); // Allow time for sensor to stabilize

  setFullScaleRange(accelRangeSetting, gyroRangeSetting);
}

void setFullScaleRange(int accelRange, int gyroRange) {
  // Set accelerometer range
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_CONFIG);
  Wire.write(accelRange << 3); // Write the selected range to the ACCEL_CONFIG register
  Wire.endTransmission(true);

  // Set gyroscope range
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(GYRO_CONFIG);
  Wire.write(gyroRange << 3); // Write the selected range to the GYRO_CONFIG register
  Wire.endTransmission(true);

  // Set the scaling factors for later calculations
  switch (accelRange) {
    case 0: // +/- 2g
      accel_scale = 16384.0;
      break;
    case 1: // +/- 4g
      accel_scale = 8192.0;
      break;
    case 2: // +/- 8g
      accel_scale = 4096.0;
      break;
    case 3: // +/- 16g
      accel_scale = 2048.0;
      break;
  }

  switch (gyroRange) {
    case 0: // +/- 250 dps
      gyro_scale = 131.0;
      break;
    case 1: // +/- 500 dps
      gyro_scale = 65.5;
      break;
    case 2: // +/- 1000 dps
      gyro_scale = 32.8;
      break;
    case 3: // +/- 2000 dps
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
    // Read raw accelerometer and gyroscope data
    int16_t accelXRaw = readRawData(ACCEL_XOUT_H);
    int16_t accelYRaw = readRawData(ACCEL_YOUT_H);
    int16_t accelZRaw = readRawData(ACCEL_ZOUT_H);
    int16_t gyroXRaw  = readRawData(GYRO_XOUT_H);
    int16_t gyroYRaw  = readRawData(GYRO_YOUT_H);
    int16_t gyroZRaw  = readRawData(GYRO_ZOUT_H);

    accelXSum += accelXRaw;
    accelYSum += accelYRaw;
    accelZSum += accelZRaw;
    gyroXSum += gyroXRaw;
    gyroYSum += gyroYRaw;
    gyroZSum += gyroZRaw;
    delay(2);
  }

  CalibrationData calibration;
  calibration.accelXBias = (float)accelXSum / numSamples;
  calibration.accelYBias = (float)accelYSum / numSamples;
  calibration.accelZBias = (float)accelZSum / numSamples;
  calibration.gyroXBias  = (float)gyroXSum / numSamples;
  calibration.gyroYBias  = (float)gyroYSum / numSamples;
  calibration.gyroZBias  = (float)gyroZSum / numSamples;

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
  // Read raw accelerometer and gyroscope data
  int16_t accelXRaw = readRawData(ACCEL_XOUT_H);
  int16_t accelYRaw = readRawData(ACCEL_YOUT_H);
  int16_t accelZRaw = readRawData(ACCEL_ZOUT_H);
  int16_t gyroXRaw  = readRawData(GYRO_XOUT_H);
  int16_t gyroYRaw  = readRawData(GYRO_YOUT_H);
  int16_t gyroZRaw  = readRawData(GYRO_ZOUT_H);

  // Apply calibration
  float accelXCalibrated = accelXRaw - calibration.accelXBias;
  float accelYCalibrated = accelYRaw - calibration.accelYBias;
  float accelZCalibrated = accelZRaw - calibration.accelZBias;
  float gyroXCalibrated  = gyroXRaw  - calibration.gyroXBias;
  float gyroYCalibrated  = gyroYRaw  - calibration.gyroYBias;
  float gyroZCalibrated  = gyroZRaw  - calibration.gyroZBias;

  // Convert to physical units using the pre-calculated scaling factors
  float accelX_g = accelXCalibrated / accel_scale;
  float accelY_g = accelYCalibrated / accel_scale;
  float accelZ_g = accelZCalibrated / accel_scale;
  float gyroX_dps = gyroXCalibrated / gyro_scale;
  float gyroY_dps = gyroYCalibrated / gyro_scale;
  float gyroZ_dps = gyroZCalibrated / gyro_scale;

  // Print the calibrated data
  Serial.print("Calibrated Accel (g): ");
  Serial.print(accelX_g);  Serial.print(", ");
  Serial.print(accelY_g);  Serial.print(", ");
  Serial.println(accelZ_g);
  Serial.print("Calibrated Gyro (dps): ");
  Serial.print(gyroX_dps); Serial.print(", ");
  Serial.print(gyroY_dps); Serial.print(", ");
  Serial.println(gyroZ_dps);

  delay(100);
}

int16_t readRawData(int address) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 2, true);
  return (Wire.read() << 8) | Wire.read();
}
