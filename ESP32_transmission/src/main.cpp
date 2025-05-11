#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoBLE.h>
#include <QMC5883LCompass.h>


// Create MPU6050 object
Adafruit_MPU6050 mpu;
QMC5883LCompass compass;

// Define service and characteristic UUIDs
const char* SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* ACCEL_UUID   = "19B10001-E8F2-537E-4F6C-D104768A1214";
const char* GYRO_UUID    = "19B10002-E8F2-537E-4F6C-D104768A1215";
const char* MAGN_UUID    = "19B10002-E8F2-537E-4F6C-D104768A1216";
const char* TEMP_UUID    = "19B10003-E8F2-537E-4F6C-D104768A1217";

struct Acceleration {
  float ax;
  float ay;
  float az;
};

struct Rotation {
  float rx;
  float ry;
  float rz;
};

struct Magnetometer {
  int azimuth;
};

struct Temperature {
  float temp;
};

// Create a BLE Service and BLE String Characteristics for sensor data
BLEService sensorService(SERVICE_UUID);
BLETypedCharacteristic<Acceleration> accelerationCharacteristic(ACCEL_UUID, BLERead | BLENotify);
BLETypedCharacteristic<Rotation> rotationCharacteristic(GYRO_UUID, BLERead | BLENotify);
BLETypedCharacteristic<Magnetometer> magnetometerCharacteristic(MAGN_UUID, BLERead | BLENotify);
BLETypedCharacteristic<Temperature> temperatureCharacteristic(TEMP_UUID, BLERead | BLENotify);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial);  

  Serial.println("Initialize System");

  // Initialize MPU6050 sensor
  if (!mpu.begin(0x68)) { // warning: the address migh be different
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("Initialized MPU6050");

  mpu.setI2CBypass(true);

  // init the QMC5883L magnetometer
  compass.init();
  // TODO: calibration of the compass
  Serial.println("Initialized QMC5883L magnetometer");


  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Erreur démarrage BLE");
    while (1);
  }
  BLE.setLocalName("MonArduinoBLE");
  BLE.setAdvertisedService(sensorService);

  // Add the three characteristics to the service
  sensorService.addCharacteristic(accelerationCharacteristic);
  sensorService.addCharacteristic(rotationCharacteristic);
  sensorService.addCharacteristic(temperatureCharacteristic);
  sensorService.addCharacteristic(magnetometerCharacteristic);
  BLE.addService(sensorService);

  // Start advertising the BLE service
  BLE.advertise();
  Serial.println("BLE started and waiting for connexion...");
}

void loop() {
  // Listen for BLE central connections
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to : ");
    Serial.println(central.address());

    // While the central is connected, continuously read sensor data and notify
    while (central.connected()) {
      // Read sensor data from the MPU6050
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      compass.read();
      int azimuth = compass.getAzimuth();

      const Acceleration accelBuffer = { a.acceleration.x, a.acceleration.y, a.acceleration.z };
      const Rotation gyroBuffer = { g.gyro.x, g.gyro.y, g.gyro.z };
      const Temperature tempBuffer = { temp.temperature };
      const Magnetometer magnBuffer = { azimuth };

      // Write (notify) the new sensor data to the respective BLE characteristics
      accelerationCharacteristic.writeValue(accelBuffer);
      rotationCharacteristic.writeValue(gyroBuffer);
      temperatureCharacteristic.writeValue(tempBuffer);
      magnetometerCharacteristic.writeValue(magnBuffer);
    }
    Serial.println("Disconnected");
  }
}

