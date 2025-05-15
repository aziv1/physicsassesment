#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "MPU6050.h"

MPU6050 mpu;

// SPI pins for SD card
int miso = 2;
int mosi = 3;
int sck  = 4;
int cs   = 10;

// I2C pins for MPU6050
int sda = 8;
int scl = 9;

// Sampling rate in milliseconds (25ms = 40Hz)
const unsigned long sampleInterval = 25;
unsigned long lastSampleTime = 0;

File dataFile;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // I2C for MPU6050
  Wire.begin(sda, scl);
  mpu.initialize();

  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  Serial.println("Accelerometer range set to +/- 16g");

  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);
  Serial.println("Gyroscope range set to +/- 500 deg/s");

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not connected!");
    while (1);
  }

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

    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Convert raw data with new scaling factors
    float accelScale = 9.80665 / 2048.0; // ±16g (16384 / 2 = 8192 counts/g, so 32768 / 16 = 2048 counts/g)
    float gyroScale = 1.0 / 65.5;       // ±500 deg/s (131 * 500 / 250 = 262, so 32768 / 500 = 65.536)

    float ax_m2 = ax * accelScale;
    float ay_m2 = ay * accelScale;
    float az_m2 = az * accelScale;

    float gx_dps = gx * gyroScale;
    float gy_dps = gy * gyroScale;
    float gz_dps = gz * gyroScale;

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
}
