#include <Arduino.h>
#include <FreeRTOSConfig.h>
#include <vector>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <esp_task_wdt.h>
#include <esp_sleep.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

using namespace std; // For coding convenience

TaskHandle_t Serial_Com_Handler; // Handler for incoming serial communication task
TaskHandle_t Check_Clim_Handler; // Handler for climate checking task

/*----------------------FOR FIREBASE DATA LOGGING -----------------------------*/

// Please update your network credentials
const char *ssid = "nono";
const char *password = "iamRaedrus";

// Insert Firebase project API Key
#define API_KEY "AIzaSyAKJmoPN1O9m2nqh40PAfdu-5Le0bnkmGk"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "zhixuanchew0915@1utar.my"
#define USER_PASSWORD "zhixuan0915"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://iot-esp32-71100-default-rtdb.asia-southeast1.firebasedatabase.app/"

/*--------------------------FIREBASE INITIALISATION----------------------------*/

// Define Firebase objects
FirebaseData stream;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Variables to save database paths
// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath1 = "/temperature1";
String humPath1 = "/humidity1";
String timePath1 = "/timestamp/.sv";

String tempPath2 = "/temperature2";
String humPath2 = "/humidity2";
String timePath2 = "/timestamp/.sv";

String timePath3 = "/timestamp/.sv";
String timePath4 = "/timestamp/.sv";
String timePath5 = "/timestamp/.sv";
String timePath6 = "/timestamp/.sv";
String timePath7 = "/timestamp/.sv";
String timePath8 = "/timestamp/.sv";

String salesChic_ID = "/ID3";
String salesPig_ID = "/ID6";
String recoveryChic_ID = "/ID4";
String recoveryPig_ID = "/ID7";
String deceasedChic_ID = "/ID5";
String deceasedPig_ID = "/ID8";

// Path to listen for changes
String listenerPath;

// Path to outputs
String outputPath;

// Sensor path (where we'll save our readings)
String sensorPath1;
String sensorPath2;

String deceasedChic_Path;
String deceasedPig_Path;
String recoveryChic_Path;
String recoveryPig_Path;
String salesChic_Path;
String salesPig_Path;

// Input paths
String inputPath;

// JSON objects to save sensor readings and timestamp
FirebaseJson json1;
FirebaseJson json2;
FirebaseJson json3;
FirebaseJson json4;
FirebaseJson json5;
FirebaseJson json6;
FirebaseJson json7;
FirebaseJson json8;

// Initialize WiFi
void initWiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
  Serial.println();
}

/*-----------------PLS CHANGE ACCORDING TO YOUR NEEDS------------------*/
// IO pins
/*INPUT*/

/*Temperature And Humidity Sensor*/
const int Temp_Sensor1 = 23; // DHT11
const int Temp_Sensor2 = 22; // DHT11

// initialise temp sensor 1 and 2 pins
#define DHTTYPE DHT11
DHT dht1(Temp_Sensor1, DHTTYPE);
DHT dht2(Temp_Sensor2, DHTTYPE);

/*Resources Consumption Sensor*/
const int Water_sensor = 32; // Potentiometer
const int Feed_sensor1 = 33; //
const int Feed_sensor2 = 34; //
const int Feed_sensor3 = 35; //
const int Feed_sensor4 = 36; //
const int Feed_sensor5 = 39; //

const int CHECKPIN = 21;
/*OUTPUT*/
/*Temperature And Humidity*/
struct control_pins
{
  const int Venti;
  const int Heater;
  const int Cooler;
} en1 = {19, 5, 18}, en2 = {4, 15, 2};
// control_pins en1 = {4, 5, 6};
// control_pins en2 = {7, 8, 9};

// const int Venti1 = 4;           //YelLED
// const int Heater1 = 12;           //RedLED
// const int Cooler1 = 4;            //BlueLED
// const int Venti2 = 4;           //YelLED
// const int Heater2 = 12;           //RedLED
// const int Cooler2 = 4;            //BlueLED

/*SERIAL COMMUNICATION*/
const int rx_pin = 16; // RX pin
const int tx_pin = 17; // TX pin

/*------------------------Variables----------------------------*/
unsigned long previous_time; // Record previous time point

// count > 6 (60s), then log data once
int tensecondscount = 0;

// Initialise Temperature and Humidity values
float Temp1 = 0; // Temperature for enclosure 1
float Humi1 = 0; // Humidity for enclosure 1
float Temp2 = 0; // Temperature for enclosure 2
float Humi2 = 0; // Humidity for enclosure 2

// UNUSED SECTION (xyyx)
bool ventilation = false;
bool cooler = false;
bool heater = false;

/*Enumerations to match data coming from ESP2*/
enum ANIMAL
{
  Pig,
  Chicken
};
ANIMAL ani;
ANIMAL enclosure;

enum STATE
{
  SalesDelivery,
  Recovery,
  Deceased
};
STATE reg;

int ID;

enum CONTROL
{
  VentilationOn,
  VentilationOff,
  CoolerOn,
  CoolerOff,
  HeaterOn,
  HeaterOff
};
CONTROL cont;

/*Internal Variables*/
String received_msg;
String received_animal;
String received_ID;

/*Structures*/
// Vectors are used due to unknown number of data being stored.
struct registry
{
  vector<int> animal_type; // Array to store the types of animals (e.g., "Pig", "Chicken")
  vector<int> ID;          // Array to store the unique IDs of animals
  int count = 0;           // Number of animals registered for delivery
} salesdelivery, recovery, deceased;

struct climate_data
{
  vector<int> temp;
  vector<int> hum;
} enclosure1, enclosure2;

/*-----------------------------FIREBASE DATABASE UPDATES----------------------------------*/

// Write float values to the database
void sendFloat(String path, float value)
{
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value))
  {
    Serial.print("Writing value: ");
    Serial.print(value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

// Write integer values to the database
void sendInt(String path, int value)
{
  if (Firebase.RTDB.setInt(&fbdo, path.c_str(), value))
  {
    Serial.print("Writing value: ");
    Serial.print(value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

// Callback function that runs on database changes
void streamCallback(FirebaseStream data)
{
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();

  // Get the path that triggered the function
  String streamPath = String(data.dataPath());

  /* When it first runs, it is triggered on the root (/) path and returns a JSON with all key
  and values of that path.So, we can get all values from the database and update the GPIO
  states.*/
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json)
  {
    FirebaseJson *json = data.to<FirebaseJson *>();
    FirebaseJsonData result;
    // Output pins
    if (json->get(result, "/digital/" + String(en1.Venti), false))
    {
      bool state = result.to<bool>();
      digitalWrite(en1.Venti, state);
    }
    if (json->get(result, "/digital/" + String(en1.Cooler), false))
    {
      bool state = result.to<bool>();
      digitalWrite(en1.Cooler, state);
    }
    if (json->get(result, "/digital/" + String(en1.Heater), false))
    {
      bool state = result.to<bool>();
      digitalWrite(en1.Heater, state);
    }
    if (json->get(result, "/digital/" + String(en2.Venti), false))
    {
      bool state = result.to<bool>();
      digitalWrite(en2.Venti, state);
    }
    if (json->get(result, "/digital/" + String(en2.Cooler), false))
    {
      bool state = result.to<bool>();
      digitalWrite(en2.Cooler, state);
    }
    if (json->get(result, "/digital/" + String(en2.Heater), false))
    {
      bool state = result.to<bool>();
      digitalWrite(en2.Heater, state);
    }
  }

  // Check for changes in the digital output values
  if (streamPath.indexOf("/digital/") >= 0)
  {
    // Get string path lengh
    int stringLen = streamPath.length();
    // Get the index of the last slash
    int lastSlash = streamPath.lastIndexOf("/");
    // Get the GPIO number (it's after the last slash "/")
    // UsersData/<uid>/outputs/digital/<gpio_number>
    String gpio = streamPath.substring(lastSlash + 1, stringLen);
    Serial.print("DIGITAL GPIO: ");
    Serial.println(gpio);
    // Get the data published on the stream path (it's the GPIO state)
    if (data.dataType() == "int")
    {
      bool gpioState = data.intData();
      Serial.print("VALUE: ");
      Serial.println(gpioState);
      // Update GPIO state
      digitalWrite(gpio.toInt(), gpioState);
    }
    Serial.println();
  }
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

/*-------------------------------------END OF FIREBASE DATABASE UPDATES---------------------------------*/

/*-------------------------------------TASKS AND FUNCTIONS---------------------------------*/

// Function to send message to esp2 and wait for its acknowledgement.
// Takes in the message that is desired to be sent in string format.
// USED FOR SENDING CLIMATE DATA TO BE DISPLAYED AT ESP2'S LCD.
void serial_wait(String msg)
{
  int c;
  Serial2.print(msg);  // Send desired message to esp2
  Serial.println(msg); // For debugging.
  received_msg = Serial2.readString();

  //  Checks for acknoledgement from esp2 and if it is busy.
  // To allocate some time for esp2 to respond in case it is currently busy.
  while (received_msg != "OKTemp" && digitalRead(CHECKPIN) == LOW)
  {
    esp_task_wdt_reset();                // Reset watchdog timer.
    received_msg = Serial2.readString(); // Read the message.
    c++;                                 // Increment count
    if (c > 400)                         // Resends the message after some time.
    {
      c = 0;
      Serial2.print(msg); // Resend message.
      Serial.print(msg);  // For debugging.
    }
  }
}

//  Function to trigger the esp2 to send data.
void serial_fetch()
{
  Serial2.flush();
  Serial2.print("request"); // Send request message.

  while (Serial2.available() == 0) // Wait for serial data.
  {
    esp_task_wdt_reset(); // Reset watchdog timer
  }
  received_msg = Serial2.readString(); // Read and trim the data.
  received_msg.trim();
}

// Function to control climate based on temperature and humidity readings.
// The temperature to trigger each control can be adjusted to meet needs.
void clim_control()
{
  /*----Enclosure 1----*/
  // Temperature
  if (Temp1 >= 30) // Check if temperature satisfy condition.
  {
    digitalWrite(en1.Cooler, HIGH); // Turn on cooler in the selected enclosure.
    sendInt(outputPath + "4", 1);   // Update output status to firebase.
  }
  else if (25 < Temp1 < 30) // Check if temperature satisfy condition.
  {
    digitalWrite(en1.Cooler, LOW); // Turn off cooler in the selected enclosure.
    sendInt(outputPath + "4", 0);  // Update output status to firebase.

    digitalWrite(en1.Heater, LOW); // Turn off heater in the selected enclosure.
    sendInt(outputPath + "2", 0);  // Update output status to firebase.
  }
  else
  {
    digitalWrite(en1.Heater, HIGH); // Turn on heater in the selected enclosure.
    sendInt(outputPath + "2", 1);   // Update output status to firebase.
  }
  // Humidity
  if (Humi1 >= 70) // Check if temperature satisfy condition.
  {
    digitalWrite(en1.Venti, HIGH); // Turn on ventilation in the selected enclosure.
    sendInt(outputPath + "15", 1); // Update output status to firebase.
  }
  else
  {
    digitalWrite(en1.Venti, LOW);  // Turn off ventilation in the selected enclosure.
    sendInt(outputPath + "15", 0); // Update output status to firebase.
  }

  /*----Enclosure 1----*/
  // Temperature
  if (Temp2 >= 30) // Check if temperature satisfy condition.
  {
    digitalWrite(en2.Cooler, HIGH); // Turn off cooler in the selected enclosure.
    sendInt(outputPath + "19", 1);  // Update output status to firebase.
  }
  else if (25 < Temp2 < 30) // Check if temperature satisfy condition.
  {
    digitalWrite(en2.Cooler, LOW); // Turn off cooler in the selected enclosure.
    sendInt(outputPath + "19", 0); // Update output status to firebase.

    digitalWrite(en2.Heater, LOW); // Turn off heater in the selected enclosure.
    sendInt(outputPath + "18", 0); // Update output status to firebase.
  }
  else
  {
    digitalWrite(en2.Heater, HIGH); // Turn on heater in the selected enclosure.
    sendInt(outputPath + "18", 1);  // Update output status to firebase.
  }
  // Humidity
  if (Humi2 >= 70) // Check if temperature satisfy condition.
  {
    digitalWrite(en2.Venti, HIGH); // Turn on ventilation in the selected enclosure.
    sendInt(outputPath + "5", 1);  // Update output status to firebase.
  }
  else
  {
    digitalWrite(en2.Venti, LOW); // Turn off ventilation in the selected enclosure.
    sendInt(outputPath + "5", 0); // Update output status to firebase.
  }
}

// Manual control of enclosure.
// Takes in enclosure selection and control type received from the keypad at esp2.
void user_clim_control(control_pins en_number, int control_type)
{

  switch (control_type)
  { // Check which control type is desired.
  case (VentilationOn):
    digitalWrite(en_number.Venti, HIGH);              // Turn on ventilation in the selected enclosure.
    sendInt(outputPath + String(en_number.Venti), 1); // Update output status to firebase.
    break;
  case (VentilationOff):
    digitalWrite(en_number.Venti, LOW);               // Turn off ventilation in the selected enclosure.
    sendInt(outputPath + String(en_number.Venti), 0); // Update output status to firebase.
    break;
  case (CoolerOn):
    digitalWrite(en_number.Cooler, HIGH);              // Turn on cooler in the selected enclosure.
    sendInt(outputPath + String(en_number.Cooler), 1); // Update output status to firebase.
    break;
  case (CoolerOff):
    digitalWrite(en_number.Cooler, LOW);               // Turn off cooler in the selected enclosure.
    sendInt(outputPath + String(en_number.Cooler), 0); // Update output status to firebase.
    break;
  case (HeaterOn):
    digitalWrite(en_number.Heater, HIGH);              // Turn on heater in the selected enclosure.
    sendInt(outputPath + String(en_number.Heater), 1); // Update output status to firebase.
    break;
  case (HeaterOff):
    digitalWrite(en_number.Heater, LOW);               // Turn off heater in the selected enclosure.
    sendInt(outputPath + String(en_number.Heater), 0); // Update output status to firebase.
    break;
  default:
    break;
  }
}

// Registry update function.
// Updates the registery specified by x
void registry_update(registry *x)
{

  Serial2.print("received"); // Message to trigger esp2 to start sending in data.
  while (Serial2.available() == 0)
  { // Wait for incoming data.
    vTaskDelay(1);
  }
  received_animal = Serial2.readString(); // Read and trim the data.
  received_animal.trim();
  x->animal_type.push_back(received_animal.toInt()); // Store animal type data into the respective enum instance.

  Serial2.print("received"); // Message to acknowledge successful transmission and request for next data.
  while (Serial2.available() == 0)
  { // Wait for incoming data.
    vTaskDelay(1);
  }
  received_ID = Serial2.readString(); // Read and trim the data.
  received_ID.trim();
  x->ID.push_back(received_ID.toInt()); // Store animal ID data into the respective enum instance.
  Serial2.print("received");            // Message to acknowledge successful transmission.

  x->count++;

  // Log registeration data to firebase based on registeration type.
  // Registeration type was already stored before the current function was called.
  switch (reg)
  {
  case SalesDelivery:
    if (received_animal.toInt() == 0) // Check if the registeration is for pig.
    {
      json6.set(salesPig_ID.c_str(), String(received_ID)); // Store the ID.
      json6.set(timePath6, "timestamp");
      Firebase.RTDB.pushJSON(&fbdo, salesPig_Path.c_str(), &json6);
    }
    else if (received_animal.toInt() == 1) // Check if the registeration is for chicken.
    {
      json3.set(salesChic_ID.c_str(), String(received_ID)); // Store the ID
      json3.set(timePath3, "timestamp");
      Firebase.RTDB.pushJSON(&fbdo, salesChic_Path.c_str(), &json3);
    }
    break;

  case Recovery:
    if (received_animal.toInt() == 0) // Check if the registeration is for pig.
    {
      json7.set(recoveryPig_ID.c_str(), String(received_ID)); // Store the ID
      json7.set(timePath7, "timestamp");
      Firebase.RTDB.pushJSON(&fbdo, recoveryPig_Path.c_str(), &json7);
    }
    else if (received_animal.toInt() == 1) // Check if the registeration is for chicken.
    {
      json4.set(recoveryChic_ID.c_str(), String(received_ID)); // Store the ID
      json4.set(timePath4, "timestamp");
      Firebase.RTDB.pushJSON(&fbdo, recoveryChic_Path.c_str(), &json4);
    }
    break;

  case Deceased:
    if (received_animal.toInt() == 0) // Check if the registeration is for pig.
    {
      json8.set(deceasedPig_ID.c_str(), String(received_ID)); // Store the ID
      json8.set(timePath8, "timestamp");
      Firebase.RTDB.pushJSON(&fbdo, deceasedPig_Path.c_str(), &json8);
    }
    else if (received_animal.toInt() == 1) // Check if the registeration is for chicken.
    {
      json5.set(deceasedChic_ID.c_str(), String(received_ID)); // Store the ID
      json5.set(timePath5, "timestamp");
      Firebase.RTDB.pushJSON(&fbdo, deceasedChic_Path.c_str(), &json5);
    }
    break;
  }
}

// LCD display update function.
// Update Temperature and Humidity display at LCD.
void LCD_Serial()
{
  Serial2.flush();
  if (!digitalRead(CHECKPIN)) // Check if esp2 is busy
  {
    serial_wait("Clim"); // Send a trigger message to let esp2 enter LCD update mode.
    Serial2.flush();

    // Convert the climate data to string and send.
    static String data_send_str;
    data_send_str = String(Temp1);
    serial_wait(data_send_str); // Send the data.

    data_send_str = String(Humi1);
    serial_wait(data_send_str); // Send the data.

    data_send_str = String(Temp2);
    serial_wait(data_send_str); // Send the data.

    data_send_str = String(Humi2);
    serial_wait(data_send_str); // Send the data.
  }
}

// Handles incoming registeration data and manual climate control.
void Serial_Com(void *pvParameters)
{

  while (1)
  {
    // Check for incoming serial message.
    if (Serial2.available() > 1)
    {
      received_msg = Serial2.readString(); // Store and trim the message.
      received_msg.trim();
      if (received_msg == "OKTemp") // Check if the message is matching
        break;                      // Stop following checks
      else
      {
        if (received_msg == "Register") // Check if the message is matching
        {
          serial_fetch();                                 // Trigger esp2 to send data
          reg = static_cast<STATE>(received_msg.toInt()); // Store and process the integer from string.
          switch (reg)                                    // Check matching registeration states/type.
          {
          case SalesDelivery:
            registry_update(&salesdelivery); // update registry of Sales and Delivery.
            break;
          case Recovery:
            registry_update(&recovery); // update registry of Recovery.
            break;
          case Deceased:
            registry_update(&deceased); // update registry of Deceased.
            break;
          default:
            break;
          }
          Serial2.print("received"); // Acknowledge and end transmission
        }
        if (received_msg == "Control") // Check if message matches.
        {
          serial_fetch();                                        // Trigger esp2 to send data
          enclosure = static_cast<ANIMAL>(received_msg.toInt()); // Store and process the integer from string.
          serial_fetch();                                        // Trigger esp2 to send data
          cont = static_cast<CONTROL>(received_msg.toInt());     // Store and process the integer from string.
          if (enclosure == Pig)                                  // Check if the integer corresponds to the enum member Pig.
          {
            user_clim_control(en1, cont); // Executes selected control in the selected enclosure.
          }

          if (enclosure == Chicken) // Check if the integer corresponds to the enum member Chicken.
          {
            user_clim_control(en2, cont); // Executes selected control in the selected enclosure.
          }
          Serial2.print("received"); // Acknowledge and end transmission
        }
      }
    }
    vTaskDelay(10);
  }
}

// Function for data logging to firebase and local storage.
void Logger()
{

  // Set temperature and humidity readings in JSON1
  json1.set(tempPath1.c_str(), String(Temp1));
  json1.set(humPath1.c_str(), String(Humi1));
  json1.set(timePath1, "timestamp");

  // Set temperature and humidity readings in JSON2
  json2.set(tempPath2.c_str(), String(Temp2));
  json2.set(humPath2.c_str(), String(Humi2));
  json2.set(timePath2, "timestamp");

  // Push JSON1 to Firebase
  Serial.printf("Set json1... %s\n", Firebase.RTDB.pushJSON(&fbdo, sensorPath1.c_str(), &json1) ? "ok" : fbdo.errorReason().c_str());

  // Push JSON2 to Firebase
  Serial.printf("Set json2... %s\n", Firebase.RTDB.pushJSON(&fbdo, sensorPath2.c_str(), &json2) ? "ok" : fbdo.errorReason().c_str());

  // For serial update via USB to PC
  Serial.print(Temp1);
  Serial.print(",");
  Serial.print(Humi1);
  Serial.print(",");
  Serial.print(Temp2);
  Serial.print(",");
  Serial.println(Humi2); // End with a newline character.
}

// Check and log climate, feed level and water level data.
void Check_Clim(void *pvParameters)
{

  while (1)
  {
    // Serial.print("Check_Temp running on core ");
    // Serial.println(xPortGetCoreID());

    // fetch temperature and hum from sensor code;
    Humi1 = dht1.readHumidity();
    Temp1 = dht1.readTemperature();
    Humi2 = dht2.readHumidity();
    Temp2 = dht2.readTemperature();

    // Set values to 0 if sensor cannot be read from.
    if (isnan(Humi1) || isnan(Temp1))
    {
      Serial.println(F("Failed to read from DHT sensor 1!"));
      Humi1 = 0;
      Temp1 = 0;
    }
    if (isnan(Humi2) || isnan(Temp2))
    {
      Serial.println(F("Failed to read from DHT sensor 2!"));
      Humi2 = 0;
      Temp2 = 0;
    }

    // Read values from potentiometers (resource consumption sensors)
    float ADCwaterLevel = analogRead(Water_sensor); // Read from pin 32
    float ADCfeedLevel1 = analogRead(Feed_sensor1); // Read from pin 33
    float ADCfeedLevel2 = analogRead(Feed_sensor2); // Read from pin 34
    float ADCfeedLevel3 = analogRead(Feed_sensor3); // Read from pin 35
    float ADCfeedLevel4 = analogRead(Feed_sensor4); // Read from pin 36
    float ADCfeedLevel5 = analogRead(Feed_sensor5); // Read from pin 39

    // Convert ADC values to percentage
    float waterLevel = ADCwaterLevel / 4100 * 100; // Read from pin 32
    float feedLevel1 = ADCfeedLevel1 / 4100 * 100; // Read from pin 33
    float feedLevel2 = ADCfeedLevel2 / 4100 * 100; // Read from pin 34
    float feedLevel3 = ADCfeedLevel3 / 4100 * 100; // Read from pin 35
    float feedLevel4 = ADCfeedLevel4 / 4100 * 100; // Read from pin 36
    float feedLevel5 = ADCfeedLevel5 / 4100 * 100; // Read from pin 39

    // Set potentiometer readings in Firebase
    sendFloat(inputPath + "32", waterLevel); // Water sensor reading
    sendFloat(inputPath + "33", feedLevel1); // Feed sensor 1 reading
    sendFloat(inputPath + "34", feedLevel2); // Feed sensor 2 reading
    sendFloat(inputPath + "35", feedLevel3); // Feed sensor 3 reading
    sendFloat(inputPath + "36", feedLevel4); // Feed sensor 4 reading
    sendFloat(inputPath + "39", feedLevel5); // Feed sensor 5 reading

    // Log data to cloud.
    Logger();

    // Control the climate based on current climate condition.
    clim_control();

    // Update LCD display with latest climate data.
    LCD_Serial();

    // Suspend the task for 1 minute.
    // Data update and logging happens every 1 minute.
    vTaskDelay(60000);
  }
}

// Setup for ESP32
void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);
  initWiFi();

  esp_task_wdt_init(30, true); // Set watchdog timeout to 30 seconds

  dht1.begin();
  dht2.begin();

  // Initialize outputs
  pinMode(en1.Venti, OUTPUT);
  pinMode(en1.Heater, OUTPUT);
  pinMode(en1.Cooler, OUTPUT);
  pinMode(en2.Venti, OUTPUT);
  pinMode(en2.Heater, OUTPUT);
  pinMode(en2.Cooler, OUTPUT);

  // Initialize all outputs to low.
  digitalWrite(en1.Venti, LOW);
  digitalWrite(en1.Heater, LOW);
  digitalWrite(en1.Cooler, LOW);
  digitalWrite(en2.Venti, LOW);
  digitalWrite(en2.Heater, LOW);
  digitalWrite(en2.Cooler, LOW);

  // Initialize inputs
  pinMode(Temp_Sensor1, INPUT);
  pinMode(Temp_Sensor2, INPUT);
  pinMode(Feed_sensor1, INPUT);
  pinMode(Feed_sensor2, INPUT);
  pinMode(Feed_sensor3, INPUT);
  pinMode(Feed_sensor4, INPUT);
  pinMode(Feed_sensor5, INPUT);
  pinMode(Water_sensor, INPUT);

  // Pin for checking status of esp2, high trigger pin (detects high as trigger).
  // If this pin receives high signal from esp2, the esp2 is busy.
  pinMode(CHECKPIN, INPUT_PULLDOWN);

  /*---------------------------------FIREBASE SETUP-----------------------------------*/

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "")
  {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path with user UID
  databasePath = "/UsersData/" + uid;

  // Update database path for listening
  listenerPath = databasePath + "/outputs/";

  // Update database path for output
  outputPath = databasePath + "/outputs/digital/";

  // Update database path for temperature sensor readings
  sensorPath1 = databasePath + "/sensor1/";
  sensorPath2 = databasePath + "/sensor2/";

  // Update database path for inputs
  inputPath = databasePath + "/inputs/";

  // Update database path for animals registration
  deceasedChic_Path = databasePath + "/chicken/deceased/";
  deceasedPig_Path = databasePath + "/pig/deceased/";
  recoveryChic_Path = databasePath + "/chicken/recovery/";
  recoveryPig_Path = databasePath + "/pig/recovery/";
  salesChic_Path = databasePath + "/chicken/sales/";
  salesPig_Path = databasePath + "/pig/sales/";

  // Streaming (whenever data changes on a path)
  // Begin stream on a database path --> UsersData/<user_uid>/outputs
  if (!Firebase.RTDB.beginStream(&stream, listenerPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  // Assign a calback function to run when it detects changes on the database
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  /*------------------------END OF FIREBASE SETUP-----------------------------------*/

  /*------------------------FREERTOS SETUP-----------------------------------*/

  // create a task that will be executed in the Serial_Com() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
      Serial_Com,           /* Task function. */
      "Serial Com Manager", /* name of task. */
      50000,                /* Stack size of task */
      NULL,                 // parameter of the task
      1,                    /* priority of the task */
      &Serial_Com_Handler,  /* Task handle to keep track of created task */
      1);                   /* pin task to core 1 */
  vTaskDelay(50);

  // create a task that will be executed in the Check_Temp() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Check_Clim,          /* Task function. */
      "Check Climate",     /* name of task. */
      50000,               /* Stack size of task */
      NULL,                /* parameter of the task */
      2,                   /* priority of the task */
      &Check_Clim_Handler, /* Task handle to keep track of created task */
      0);                  /* pin task to core 0 */
  vTaskDelay(50);
}

/*------------------------END OF FREERTOSS SETUP-----------------------------------*/

void loop()
{
}

/*
----------------------Partially Done Codes------------------------------
 
  // for Saving the unlogged data when Wi-Fi is unavailable. When it becomes available again, all the saved data will be logged to the cloud
  {received_msg2 = Serial2.readString();
  received_msg2.trim();
  int id_received = received_msg2.toInt();
  x->ID.push_back(id_received);
  Serial2.print("received");

  x->count++;

  // Check if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    // If there was previously offline data, send it first
    if (isOffline && !pending.animal_type.empty()) {
      for (int i = 0; i < pending.animal_type.size(); i++) {
        int type = pending.animal_type[i];
        int id = pending.ID[i];

        // Send offline data to Firebase
        switch (reg) {
          case SalesDelivery:
            if (type == 0) { // Pig
              json6.set(salesPig_ID.c_str(), String(id));
              json6.set(timePath6, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, salesPig_Path.c_str(), &json6);
            } else if (type == 1) { // Chicken
              json3.set(salesChic_ID.c_str(), String(id));
              json3.set(timePath3, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, salesChic_Path.c_str(), &json3);
            }
            break;

          case Recovery:
            if (type == 0) { // Pig
              json7.set(recoveryPig_ID.c_str(), String(id));
              json7.set(timePath7, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, recoveryPig_Path.c_str(), &json7);
            } else if (type == 1) { // Chicken
              json4.set(recoveryChic_ID.c_str(), String(id));
              json4.set(timePath4, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, recoveryChic_Path.c_str(), &json4);
            }
            break;

          case Deceased:
            if (type == 0) { // Pig
              json8.set(deceasedPig_ID.c_str(), String(id));
              json8.set(timePath8, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, deceasedPig_Path.c_str(), &json8);
            } else if (type == 1) { // Chicken
              json5.set(deceasedChic_ID.c_str(), String(id));
              json5.set(timePath5, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, deceasedChic_Path.c_str(), &json5);
            }
            break;
        }
      }

      // Clear pending data after successful sync
      pending.animal_type.clear();
      pending.ID.clear();
      pending.count = 0;
      isOffline = false;
    }

    // Send current data to Firebase
    switch (reg) {
      case SalesDelivery:
        if (animal_type_received == 0) { // Pig
          json6.set(salesPig_ID.c_str(), String(id_received));
          json6.set(timePath6, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, salesPig_Path.c_str(), &json6);
        } else if (animal_type_received == 1) { // Chicken
          json3.set(salesChic_ID.c_str(), String(id_received));
          json3.set(timePath3, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, salesChic_Path.c_str(), &json3);
        }
        break;

      case Recovery:
        if (animal_type_received == 0) { // Pig
          json7.set(recoveryPig_ID.c_str(), String(id_received));
          json7.set(timePath7, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, recoveryPig_Path.c_str(), &json7);
        } else if (animal_type_received == 1) { // Chicken
          json4.set(recoveryChic_ID.c_str(), String(id_received));
          json4.set(timePath4, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, recoveryChic_Path.c_str(), &json4);
        }
        break;

      case Deceased:
        if (animal_type_received == 0) { // Pig
          json8.set(deceasedPig_ID.c_str(), String(id_received));
          json8.set(timePath8, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, deceasedPig_Path.c_str(), &json8);
        } else if (animal_type_received == 1) { // Chicken
          json5.set(deceasedChic_ID.c_str(), String(id_received));
          json5.set(timePath5, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, deceasedChic_Path.c_str(), &json5);
        }
        break;
    }
  } else {
    // If offline, store data locally in the 'pending' struct
    pending.animal_type.push_back(animal_type_received);
    pending.ID.push_back(id_received);
    pending.count++;
    isOffline = true;
  }
}//registry update

// to automatic control the water and feed system 
void water_system_control(){
  if (waterLevel <= 30) // Check if water level is too low.
  {
    digitalWrite(water_control, HIGH); // Open the valve to fill water
    sendInt(outputPath + String(water_control), 1);   // Update output status to firebase.
  }
  if (waterLevel >= 60) // Check if water level is sufficient.
  {
    digitalWrite(water_control, LOW); // Close the valve to stop filling water
    sendInt(outputPath + String(water_control), 0);   // Update output status to firebase.
  }
}

#include "esp_sleep.h"

// for saving power consumption by entering sleep
void sleep(){
  // Configure the ESP32 to wake up on UART (serial input)
  esp_sleep_enable_uart_wakeup(USART_NUM_2); // wake when there is activity For Serial port 2
  esp_sleep_enable_uart_wakeup(USART_NUM_1); // wake when there is activity For Serial port 1
  esp_light_sleep_start();  // Enter light sleep 
}


*/
