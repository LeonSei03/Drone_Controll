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

// globale Variablen
float gyro_offset_x = 0;
float gyro_offset_y = 0;
float gyro_offset_z = 0;

float gyro_x = 0;
float gyro_y = 0;
float gyro_z = 0;

double accel_offset_x = 0; 
double accel_offset_y = 0;
double accel_offset_z = 0;

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

  // Vairablen um die Beschleunigungen zu berechnen
  double sum_accel_x = 0;
  double sum_accel_y = 0;
  double sum_accel_z = 0;
  int counter_accel = 0;

  Serial.println("MPU Beschleunigungssensor wird Kalibriert, bitte nicht bewegen!");

  // Schleife um den Durchschnitt der Beschleunigungen im Ausgangszustand zu berechnen um zu schauen ob wir im richtigen Bereich liegen
  for(int i = 0; i < 500; i++) {
    mpu.getEvent(&accel, &gyro, &temp);

    sum_accel_x += accel.acceleration.x;
    sum_accel_y += accel.acceleration.y;
    sum_accel_z += accel.acceleration.z;

    counter_accel++;
    delay(2);
  }

  // Durchschnitte berechnen
  accel_offset_x = sum_accel_x / counter_accel;
  accel_offset_y = sum_accel_y / counter_accel;
  accel_offset_z = sum_accel_z / counter_accel;

  // Gesamt wirkende Beschleunigung, sollte nahe 9.81 m/s^2 sein
  double accel_betrag = sqrt((accel_offset_x * accel_offset_x) + (accel_offset_y * accel_offset_y) + (accel_offset_z * accel_offset_z));

  Serial.print("Betrag der Beschleunigung über x, y, z: ");
  Serial.println(accel_betrag);

  // Offsets sammeln und aufaddieren
  double sum_gyro_x = 0;
  double sum_gyro_y = 0;
  double sum_gyro_z = 0;
  int counter = 0;

  Serial.println("MPU Gyro wird Kalibriert, bitte nicht bewegen!");

  // Gyro Offset Kalibrierung -> Hier sammeln wir die ersten 1500 Werte, mitteln diese und ziehen sie nachher von den neu gemessenen Werten ab.
  for(int i = 0; i < 1500; i++) {
    mpu.getEvent(&accel, &gyro, &temp);

    sum_gyro_x += gyro.gyro.x;
    sum_gyro_y += gyro.gyro.y;
    sum_gyro_z += gyro.gyro.z;

    counter++; 
    delay(2); 
  } 

  // Offset der dann von jedem Gyro Wert abgezogen wird. Es wird also der "Fehler des Sensors" dann von den aktuellen Messwerten abgezogen 
  gyro_offset_x = sum_gyro_x / counter; 
  gyro_offset_y = sum_gyro_y / counter;
  gyro_offset_z = sum_gyro_z / counter;

  Serial.println("Kalibrierung ist fertig!");
}

void loop() {
  // aktuelle Sensorwerte holen und in die Datenbehälter reinschreiben
  mpu.getEvent(&accel, &gyro, &temp);

  // Temperatur ausgeben
  // Serial.print(" Temp: ");
  // Serial.print(temp.temperature);
  // Serial.println(" C");

  // Pitch des Sensors berechnen 
  // Beim Pitch wird um die y-Achse des Sensors gedreht, dh Erdbeschleunigung teilt sich auf z- und x-Achse auf
  double pitch_rad = atan2(accel.acceleration.x, sqrt(accel.acceleration.y * accel.acceleration.y + accel.acceleration.z * accel.acceleration.z));
  double pitch_deg = pitch_rad * 180 / PI; 

  // Roll des Sensors berechnen
  // Beim Roll dreht/rollt der Sensor quasi um die x-Achse
  double roll_rad = atan2(accel.acceleration.y, accel.acceleration.z);
  double roll_deg = roll_rad * 180 / PI; 

  // Roll und Pitch ausgeben
  Serial.print("Pitch (Drehung um y-Achse): ");
  Serial.print(pitch_deg);

  Serial.print(" | Roll (Drehung um x-Achse): ");
  Serial.print(roll_deg);

  // Beschleunigung ausgeben
  // Serial.print(" | Accel x: ");
  // Serial.print(accel.acceleration.x);
  // Serial.print(" | y: ");
  // Serial.print(accel.acceleration.y);
  // Serial.print(" | z: ");
  // Serial.print(accel.acceleration.z);
  // Serial.print(" | ");

  // Bei jeder iteration an Messungen auch den Betrag ausgeben
  double accel_betrag = sqrt((accel.acceleration.x * accel.acceleration.x) + (accel.acceleration.y * accel.acceleration.y) + (accel.acceleration.z * accel.acceleration.z));
  // Serial.print("Betrag der Beschleunigung: ");
  // Serial.print(accel_betrag);
  Serial.print(" | ");

  // Gyro Werte ausgeben (Einheit in rad/s)
  gyro_x = gyro.gyro.x - gyro_offset_x;
  gyro_y = gyro.gyro.y - gyro_offset_y;
  gyro_z = gyro.gyro.z - gyro_offset_z;

  Serial.print(" Gyro x: ");
  Serial.print(gyro_x);
  Serial.print(" | y: ");
  Serial.print(gyro_y);
  Serial.print(" | z: ");
  Serial.println(gyro_z);

  delay(500);
}