#include <Arduino.h>
#include <Wire.h>
// #include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_ICM20X.h>

const int PIN_SDA = 18;
const int PIN_SCL = 19;

// MPU Objekt anlegen
//Adafruit_MPU6050 mpu;
// ICM Objekt anlegen
Adafruit_ICM20948 icm;

// Sensor Variablen festlegen
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;
sensors_event_t mag;

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

// Variable für die Berechnung der vergangenen Zeit von der letzen Messung
unsigned long last_time = 0; 

// Variablen für Winkelberechnung über die Winkelgeschwindigkeit
double roll_gyro = 0;
double pitch_gyro = 0;

void setup() {
  // Startet die Serielle Verbindung zum PC mit einer Baudrate von 115200 -> auch im Serial Monitor einstellen
  Serial.begin(115200);

  // Kurz warten nach dem Start
  delay(100);

  Serial.println("Starte ICM 20948 Test:");

  // I2C Bus starten
  Wire.begin(PIN_SDA, PIN_SCL);
  
  // I2C langsamer machen
  Wire.setClock(10000);

  // Sensor suchen 
  if (!icm.begin_I2C(0x69, &Wire)){ // mpu.begin_I2C sucht uns den Sensor über den I2C Bus
    Serial.println("Fehler: ICM 20948 auf 0x69 nicht gefunden! Es wird 0x68 getestet");
  
    if(!icm.begin_I2C(0x68, &Wire)){
      Serial.println("Fehler: ICM 20948 auf 0x68 nicht gefunden!");
      while(true){
        delay(100);
    }
  }
  } else {
    Serial.println("ICM 20948 gefunden!");
    Serial.println("Sensorwerte werden ausgelesen...");
  }

  // Vairablen um die Beschleunigungen zu berechnen
  double sum_accel_x = 0;
  double sum_accel_y = 0;
  double sum_accel_z = 0;
  int counter_accel = 0;

  Serial.println("ICM Beschleunigungssensor wird Kalibriert, bitte nicht bewegen!");

  // Schleife um den Durchschnitt der Beschleunigungen im Ausgangszustand zu berechnen um zu schauen ob wir im richtigen Bereich liegen
  for(int i = 0; i < 500; i++) {
    icm.getEvent(&accel, &gyro, &temp, &mag);

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

  Serial.println("ICM Gyro wird Kalibriert, bitte nicht bewegen!");

  // Gyro Offset Kalibrierung -> Hier sammeln wir die ersten 1500 Werte, mitteln diese und ziehen sie nachher von den neu gemessenen Werten ab.
  for(int i = 0; i < 1500; i++) {
    icm.getEvent(&accel, &gyro, &temp, &mag);

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
  last_time = micros();
}

void loop() {
  // aktuelle Sensorwerte holen und in die Datenbehälter reinschreiben
  icm.getEvent(&accel, &gyro, &temp, &mag);

  // Temperatur ausgeben
  // Serial.print(" Temp: ");
  // Serial.print(temp.temperature);
  // Serial.println(" C");

  // Pitch des Sensors berechnen 
  // Beim Pitch wird um die y-Achse des Sensors gedreht, dh Erdbeschleunigung teilt sich auf z- und x-Achse auf
  double pitch_rad = atan2(accel.acceleration.x, sqrt(accel.acceleration.y * accel.acceleration.y + accel.acceleration.z * accel.acceleration.z));
  double pitch_deg_accel = pitch_rad * 180 / PI; 

  // Roll des Sensors berechnen
  // Beim Roll dreht/rollt der Sensor quasi um die x-Achse
  double roll_rad = atan2(accel.acceleration.y, accel.acceleration.z);
  double roll_deg_accel = roll_rad * 180 / PI; 

  // Roll und Pitch ausgeben
  // Serial.print(" Pitch (Drehung um y-Achse): ");
  // Serial.print(pitch_deg_accel);

  // Serial.print(" | Roll (Drehung um x-Achse): ");
  // Serial.print(roll_deg_accel);

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
  Serial.print(" | Betrag Accel ");
  Serial.print(accel_betrag);

  // Gyro Werte ausgeben (Einheit in rad/s)
  gyro_x = gyro.gyro.x - gyro_offset_x;
  gyro_y = gyro.gyro.y - gyro_offset_y;
  gyro_z = gyro.gyro.z - gyro_offset_z;

  // Zeit seit start des Programms
  unsigned long current_time = micros();
  double dt = (current_time - last_time) / 1000000.0;
  last_time = current_time; 

  // Winkel berechnen über die Winkelgeschwindigkeit
  roll_gyro += gyro_x * dt * 180 / PI;
  pitch_gyro += gyro_y * dt * 180 / PI; 

  // Serial.print(" | Roll über Gyro: ");
  // Serial.print(roll_gyro);
  // Serial.print(" | Pitch über Gyro: ");
  // Serial.println(pitch_gyro);

  // Serial.print(" Gyro x: ");
  // Serial.print(gyro_x);
  // Serial.print(" | y: ");
  // Serial.print(gyro_y);
  // Serial.print(" | z: ");
  // Serial.println(gyro_z);

  // Fusion der Winkel vom Gyro und vom Accel
  // zu diesen Anteil fließt der Winkel vom gyro ein
  double alpha = 0.98;  
  
  double roll_angle_fusion = alpha * roll_gyro + (1 - alpha) * roll_deg_accel;
  double pitch_angle_fusion = alpha * pitch_gyro + (1 - alpha) * pitch_deg_accel;

  Serial.print(" | Fusion Roll (um x-Achse): ");
  Serial.print(roll_angle_fusion);
  Serial.print(" | Fusion Pitch (um y-Achse): ");
  Serial.println(pitch_angle_fusion);

  delay(5);
}