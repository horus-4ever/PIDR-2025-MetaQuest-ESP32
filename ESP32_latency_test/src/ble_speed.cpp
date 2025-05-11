#include <Arduino.h>
#include <Wire.h>
#include <ArduinoBLE.h>

// Define service and characteristic UUIDs
const char* SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* TIME_UUID   = "19B10001-E8F2-537E-4F6C-D104768A1214";

// Create a BLE Service and BLE String Characteristics for sensor data
BLEService timeService(SERVICE_UUID);
BLETypedCharacteristic<unsigned long> timeCharacteristic(TIME_UUID, BLERead | BLENotify);

unsigned long last_time;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Initialize System");

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Erreur démarrage BLE");
    while (1);
  }
  BLE.setLocalName("MonArduinoBLE");
  BLE.setAdvertisedService(timeService);
  BLE.setConnectionInterval(7.25, 7.25);

  // Add the three characteristics to the service
  timeService.addCharacteristic(timeCharacteristic);
  BLE.addService(timeService);

  // Start advertising the BLE service
  BLE.advertise();
  Serial.println("BLE démarré et en attente de connexion...");
}

void loop() {
  // Listen for BLE central connections
  BLEDevice central = BLE.central();

  if (central) {
    last_time = millis();
    Serial.print("Connecté à : ");
    Serial.println(central.address());

    // While the central is connected, continuously read sensor data and notify
    while (central.connected()) {
      unsigned long time = millis();
      if(time - last_time >= 7.25) {
        timeCharacteristic.writeValue(time);
        last_time = time;
      }
    }
    Serial.println("Déconnecté");
  }
}
