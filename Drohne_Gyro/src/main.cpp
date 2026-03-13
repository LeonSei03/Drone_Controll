

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const int PIN_SDA = 33;
const int PIN_SCL = 32;

// MPU Objekt anlegen
Adafruit_MPU6050 mpu;

// Sensor Variablen festlegen
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;


void setup() {
  // Startet die Serielle Verbindung zum PC mit einer Baudrate von 115200 -> auch im Serial Monitor einstellen
  Serial.begin(115200);

  // Kurz warten nach dem Start
  delay(100);

  Serial.println("Starte MPU6050 Test:");

  // I2C Bus starten
  Wire.begin(PIN_SDA, PIN_SCL);
  
  // I2C langsamer machen
  Wire.setClock(10000);

  // Sensor suchen 
  if (!mpu.begin()){ // mpu.begin_I2C sucht uns den Sensor über den I2C Bus
    Serial.println("Fehler: MPU-6050 nicht gefunden!");
  
    while(true){
      delay(100);
    }
  } else {
    Serial.println("MPU-6050 gefunden!");
    Serial.println("Sensorwerte werden ausgelesen...");
  }
}


void loop() {
  // aktuelle Sensorwerte holen und in die Datenbehälter reinschreiben

  mpu.getEvent(&accel, &gyro, &temp);

  // Temperatur ausgeben
  Serial.print("Temp: ");
  Serial.print(temp.temperature);
  Serial.println(" C");

  // Beschleunigung ausgeben
  Serial.print("Accel x: ");
  Serial.print(accel.acceleration.x);
  Serial.print(" | y: ");
  Serial.print(accel.acceleration.y);
  Serial.print(" | z: ");
  Serial.print(accel.acceleration.z);

  // Gyro Werter ausgeben (Einheit in rad/s)
  Serial.print("Gyro x: ");
  Serial.print(gyro.gyro.x);
  Serial.print(" | y: ");
  Serial.print(gyro.gyro.y);
  Serial.print(" | z: ");
  Serial.print(gyro.gyro.z);

  // Magnetometer ausgeben

  delay(500);
}