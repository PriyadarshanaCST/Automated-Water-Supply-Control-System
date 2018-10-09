
#include <Arduino.h>
#include <EEPROM.h>
#define USE_SERIAL Serial
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define water_sensor 15
  int val;
#include <FirebaseArduino.h>
#include <ArduinoJson.h>

// Variable init
// defines pins numbers
const int trigPin = 2;  //D4 - Green Wire
const int echoPin = 0;  //D3 - White Wire

const int buttonPin = D2; // variable for D2 pin
const int buttonPin_2 = D8; // flow sensor_2
const int ledPin = D7;
char push_data[200]; //
char push_data_2[200]; //
int addr = 0; //endereço eeprom
byte sensorInterrupt = 0; // 0 = digital pin 2
byte sensorInterrupt_2 = D8; // 

// defines variables
float duration, distance, from_tank, out_1;
float height, area, max_level , min_level, water_level;
bool Well_on, WBS_on, WBS, Well;
int n = 0;
String filling_mode, prefered_source;
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;
volatile byte pulseCount_2;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

// flow sensor 2
float flowRate_2;  // flow Sensor 2
unsigned int flowMilliLitres_2; // Sensor 2
unsigned long totalMilliLitres_2; // Sensor 2

unsigned long oldTime;
unsigned long oldTime_2;

// Set WIFI & Firebase details
#define FIREBASE_HOST "autowaterss-bc262.firebaseio.com"
#define FIREBASE_AUTH "tpg084DXXaZnPswxVFFvCRv5sfOFtDyN2YbM2pZN"

//SSID and PASSWORD for the AP (swap the XXXXX for real ssid and password )
const char * ssid = "SLT-4G_107780-SCHI";
const char * password = "6095@slt";

//HTTP client init
HTTPClient http;
unsigned int frac;
unsigned int frac_2;
void setup() {
  Serial.begin(9600); // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  startWIFI();

  // Initialization of the variable “buttonPin” as INPUT (D2 pin)
  pinMode(buttonPin, INPUT);
    pinMode(buttonPin_2, INPUT);
 pinMode(water_sensor, INPUT);
  // Two types of blinking
  // 1: Connecting to Wifi
  // 2: Push data to the cloud
  pinMode(ledPin, OUTPUT);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  oldTime = 0;

// flow sensor 2
  pulseCount_2 = 0;
  flowRate_2 = 0.0;
  flowMilliLitres_2 = 0;
  totalMilliLitres_2 = 0;
  oldTime_2 = 0;


  digitalWrite(buttonPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin), pulseCounter, RISING);

  digitalWrite(buttonPin_2, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin_2), pulseCounter, RISING);
  //connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  if(Firebase.failed()){
     Serial.println("Firebase Failed!");
    }else{
       Serial.println("Firebase Connected");
      }
      
  Firebase.set("LED_STATUS", 1);

  // Define inputs and Outputs
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  digitalWrite(trigPin , LOW);
  delayMicroseconds(5);

  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D5, OUTPUT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && (millis() - oldTime) > 1000) // Only process counters once per second
  {
     val = digitalRead(water_sensor);          /// folw sensor_1//////
    detachInterrupt(sensorInterrupt);


    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount); // calibrationFactor;

    oldTime = millis();


    flowMilliLitres = (flowRate / 60) * 1000;


    totalMilliLitres += flowMilliLitres;

    unsigned int frac;

                
                /// folw sensor_2//////

    detachInterrupt(sensorInterrupt_2);


    flowRate_2 = ((1000.0 / (millis() - oldTime_2)) * pulseCount_2); // calibrationFactor;

    oldTime_2 = millis();


    flowMilliLitres_2 = (flowRate_2 / 60) * 1000;


    totalMilliLitres_2 += flowMilliLitres_2;

    unsigned int frac_2;



 

        //////////flow sensor_1/////////

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate)); // Print the integer part of the variable
    Serial.print("."); // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC); // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: "); // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: "); // Output separator
    Serial.print(totalMilliLitres);
    Serial.println("mL");
                   

        //////////flow sensor_2/////////
              
// Print the flow rate for this second in litres / minute
    Serial.print("Flow rate_2: ");
    Serial.print(int(flowRate_2)); // Print the integer part of the variable
    Serial.print("."); // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac_2 = (flowRate_2 - int(flowRate_2)) * 10;
    Serial.print(frac_2, DEC); // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: "); // Output separator
    Serial.print(flowMilliLitres_2);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: "); // Output separator
    Serial.print(totalMilliLitres_2);
    Serial.println("mL");


    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;


    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  } else if (WiFi.status() != WL_CONNECTED) {
    startWIFI();
  }
  // get value
  n = Firebase.getInt("LED_STATUS");
  height = Firebase.getFloat("water_tank/measurements/height");
  max_level = Firebase.getFloat("water_tank/measurements/max_level");
  min_level = Firebase.getFloat("water_tank/measurements/min_level");
  WBS = Firebase.getBool("water_tank/filling/Manual_WBS");
  Well = Firebase.getBool("water_tank/filling/Manual_Well");
  filling_mode = Firebase.getString("water_tank/filling/filling_mode");
  prefered_source = Firebase.getString("water_tank/filling/prefered_source");


 
 

   // push water flow
  
  StaticJsonBuffer<200> jsonBuffer1;
  JsonObject& pipeline = jsonBuffer1.createObject();
  pipeline["from_tank"] = flowRate;
  pipeline["out_1"] = "0.01";
  pipeline["water_well"] = "true";
  Firebase.push("pipeline", pipeline);
  
  // handle error
  if (n == 1) {
    digitalWrite(D7, HIGH);
    delay(10);
  } else {
    digitalWrite(D7, LOW);
  }


  // The Sensor is triggerd by a HIGH pulse of or more microseconds.
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW );

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance or Conert time into distance
  distance = (duration / 2 ) / 29.1 ; // getting cm


  // getting water level calculation
  water_level = (height - distance);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& lvl = jsonBuffer.createObject();
  lvl["level"] = water_level;

  Firebase.push("water_tank/water_level", lvl);

  //Logic
  if (filling_mode == "auto") {
    if (water_level < max_level * height) {
      if (water_level < min_level * height) {
        if (prefered_source == "WBS") {
          Firebase.setBool("water_tank/realtime_data/WBSon", true);
        } else {
          Firebase.setBool("water_tank/realtime_data/Wellon", true);
        }
      }
    } else {
      Firebase.setBool("water_tank/realtime_data/WBSon", false);
      Firebase.setBool("water_tank/realtime_data/Wellon", false);
    }
  } else {
    if (height > water_level) {
      if (WBS) {
        Firebase.setBool("water_tank/realtime_data/WBSon", true);
      } else {
        Firebase.setBool("water_tank/realtime_data/WBSon", false);
      }

      if (Well) {
        Firebase.setBool("water_tank/realtime_data/Wellon", true);
      } else {
        Firebase.setBool("water_tank/realtime_data/Wellon", false);
      }
    } else {
      Firebase.setBool("water_tank/realtime_data/WBSon", false);
      Firebase.setBool("water_tank/realtime_data/Wellon", false);
    }
  }

  //Turning on Moters
  WBS_on = Firebase.getBool("water_tank/realtime_data/WBSon");
  if (WBS_on) {
    digitalWrite(D5, HIGH);
  } else {
    digitalWrite(D5, LOW);
  }

  Well_on = Firebase.getBool("water_tank/realtime_data/Wellon");
  if (Well_on && val== HIGH) {
    digitalWrite(D6, HIGH);
    Serial.println("Motor working");
  } else {
    digitalWrite(D6, LOW);
    Serial.println("Motor not working");
  }


  // Sending to monitor
  Serial.print("height is : " );
  Serial.println(height);
  Serial.print("cm, ");

  Serial.print("Water Level :");
  Serial.print(water_level);
  Serial.print("cm, ");

  Serial.print("min Level :");
  Serial.print(min_level*height);
  Serial.print("cm, ");

  Serial.print("man Level :");
  Serial.print(max_level*height);
  Serial.print("cm, ");

  Serial.print("Sensor Reading :");
  Serial.print(distance);
  Serial.print("cm, ");

  Serial.print("height :");
  Serial.print(height);
  Serial.print("cm, ");

Serial.println(filling_mode);
Serial.println(min_level);
Serial.println(max_level);

  Serial.println();
  delay(1000);


}

/*
  Insterrupt Service Routine
*/
void pulseCounter() {
  // Increment the pulse counter
  pulseCount++;
}

void startWIFI() {
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);

  WiFi.begin(ssid, password); // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");
  oldTime = 0;
  int i = 0;
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  delay(100);

  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(2000);
    Serial.print(++i);
    Serial.print('.');
    digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
  delay(2000);
  Serial.print('\n');
  Serial.print("Connection established!");
  Serial.print("IP address:\t");
  Serial.print(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer

}
