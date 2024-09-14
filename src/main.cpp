#include <Arduino.h>
#include <FreeRTOSConfig.h>
#include <vector>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <esp_task_wdt.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


using namespace std;



TaskHandle_t Serial_Com_Handler;  //Manages incoming serial data    
TaskHandle_t Check_Clim_Handler;  // Check Temp + Climate Control
TaskHandle_t Logger_Handler;      // Log Data 
TaskHandle_t IoT_Handler;         // Check and Update for IoT 
TaskHandle_t Resources_Monitor_Handler;    // Check Feed (5) + Water Consumption

// TaskHandle_t Keypad_Check_Handler;      // Check Keypad_Check and the Input

/*----------------------FOR FIREBASE DATA LOGGING -----------------------------*/

// Please update your network credentials
const char* ssid = "nono";
const char* password = "iamRaedrus";

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
String humPath2= "/humidity2";
String timePath2 = "/timestamp/.sv";

// Path to listen for changes
String listenerPath;

// Path to outputs
String outputPath;

// Sensor path (where we'll save our readings)
String sensorPath1;
String sensorPath2;

// Input paths
String inputPath;

// JSON objects to save sensor readings and timestamp
FirebaseJson json1;
FirebaseJson json2;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
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
const int Temp_Sensor1 = 23;    //DHT11
const int Temp_Sensor2 = 22;    //DHT11

//initialise temp sensor 1 and 2 pins 
#define DHTTYPE DHT11
DHT dht1(Temp_Sensor1, DHTTYPE);
DHT dht2(Temp_Sensor2, DHTTYPE);

/*Resources Consumption Sensor*/
const int Water_sensor = 32;   //Potentiometer
const int Feed_sensor1 = 33;    //
const int Feed_sensor2 = 34;   //
const int Feed_sensor3 = 35;   //
const int Feed_sensor4 = 36;   //
const int Feed_sensor5 = 39;   //
    
/*OUTPUT*/
/*Temperature And Humidity*/
struct control_pins{
  const int Venti;
  const int Heater;
  const int Cooler;
}en1={15,2,4},en2={5,18,19};
// control_pins en1 = {4, 5, 6};
// control_pins en2 = {7, 8, 9};

// const int Venti1 = 4;           //YelLED
// const int Heater1 = 12;           //RedLED
// const int Cooler1 = 4;            //BlueLED
// const int Venti2 = 4;           //YelLED
// const int Heater2 = 12;           //RedLED
// const int Cooler2 = 4;            //BlueLED

/*SERIAL COMMUNICATION*/
const int rx_pin = 16;       //RX pin
const int tx_pin = 17;       //TX pin

/*Variables*/

//count > 6 (60s), then log data once
int tensecondscount = 0;

//Initialise Temperature and Humidity values
float Temp1=0; //Temperature for enclosure 1
float Humi1=0; //Humidity for enclosure 1
float Temp2=0; //Temperature for enclosure 2
float Humi2=0; //Humidity for enclosure 2

//UNUSED SECTION (xyyx)
bool ventilation=false; 
bool cooler=false;
bool heater=false;

/*Enumerations to match data coming from ESP2*/
enum ANIMAL {Pig, Chicken};
ANIMAL ani;
ANIMAL enclosure;

enum STATE {SalesDelivery,Recovery,Deceased};
STATE reg;

int ID;

enum CONTROL {VentilationOn, VentilationOff, CoolerOn, CoolerOff, 
HeaterOn, HeaterOff};
CONTROL cont;

/*Internal Variables*/
String received_msg;
int previous_temp;
int previous_hum;

/*Structures*/
const int max_reg=100;
struct registry {
  vector<int> animal_type; // Array to store the types of animals (e.g., "Pig", "Chicken")
  vector<int> ID;         // Array to store the unique IDs of animals
  int count=0;                      // Number of animals registered for delivery
}salesdelivery, recovery, deceased;

struct climate_data{
  vector<int> temp;
  vector<int> hum;
}enclosure1,enclosure2;

/*-----------------------------FIREBASE DATABASE UPDATES----------------------------------*/

// Write float values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

// Write integer values to the database
void sendInt(String path, int value){
  if (Firebase.RTDB.setInt(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

// Callback function that runs on database changes
void streamCallback(FirebaseStream data){
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();

  // Get the path that triggered the function
  String streamPath = String(data.dataPath());

  /* When it first runs, it is triggered on the root (/) path and returns a JSON with all key
  and values of that path.So, we can get all values from the database and updated the GPIO
  states, PWM, and message on OLED*/
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json){
    FirebaseJson *json = data.to<FirebaseJson *>();
    FirebaseJsonData result;
    //Output pins
    if (json->get(result, "/digital/" + String(en1.Venti), false)){
      bool state = result.to<bool>();
      digitalWrite(en1.Venti, state);
    }
    if (json->get(result, "/digital/" + String(en1.Cooler), false)){
      bool state = result.to<bool>();
      digitalWrite(en1.Cooler, state);
    }
    if (json->get(result, "/digital/" + String(en1.Heater), false)){
      bool state = result.to<bool>();
      digitalWrite(en1.Heater, state);
    }
    if (json->get(result, "/digital/" + String(en2.Venti), false)){
      bool state = result.to<bool>();
      digitalWrite(en2.Venti, state);
    }if (json->get(result, "/digital/" + String(en2.Cooler), false)){
      bool state = result.to<bool>();
      digitalWrite(en2.Cooler, state);
    }
    if (json->get(result, "/digital/" + String(en2.Heater), false)){
      bool state = result.to<bool>();
      digitalWrite(en2.Heater, state);
    }
  }


  // Check for changes in the digital output values
  if(streamPath.indexOf("/digital/") >= 0){
    // Get string path lengh
    int stringLen = streamPath.length();
    // Get the index of the last slash
    int lastSlash = streamPath.lastIndexOf("/");
    // Get the GPIO number (it's after the last slash "/")
    // UsersData/<uid>/outputs/digital/<gpio_number>
    String gpio = streamPath.substring(lastSlash+1, stringLen);
    Serial.print("DIGITAL GPIO: ");
    Serial.println(gpio);
    // Get the data published on the stream path (it's the GPIO state)
    if(data.dataType() == "int") {
      bool gpioState = data.intData();
      Serial.print("VALUE: ");
      Serial.println(gpioState);
      //Update GPIO state
      digitalWrite(gpio.toInt(), gpioState);
    }
    Serial.println();
  }
}

void streamTimeoutCallback(bool timeout){
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

/*-------------------------------------END OF FIREBASE DATABASE UPDATES---------------------------------*/

///////////////////////////////////////////////////////////////
/* * * * * * * * * * Tasks and Functions * * * * * * * * * * */

void LCD_Serial(){
  //Serial Com to 2nd ESP for LCD output
}

void serial_fetch(){ //to be used in Serial_Com
  delay(1000);
  Serial2.flush();
  Serial2.print("received");
          
  while (Serial2.available()==0){
    delay(10);
  }
  received_msg=Serial2.readString();
  received_msg.trim();
}

// bool check_clim_ifchange(){
//   if (prev_enclosure1.temp.back()!=enclosure1.temp.back()
//   ||prev_enclosure1.hum.back()!=enclosure1.hum.back()
//   ||prev_enclosure2.temp.back()!=enclosure2.temp.back()
//   ||prev_enclosure2.hum.back()!=enclosure2.hum.back()){
//     return true;
//   }
//   else{
//     return false;
//   }
// }



void clim_control(){

  //Enclosure 1
  if (Temp1>=30){
    sendInt(outputPath + "4", 1); //eAn1.Cooler On
  }
  else if (25<Temp1<30){
    sendInt(outputPath + "4", 0); //en1.Cooler Off
    sendInt(outputPath + "2", 0); //en1.Heater Off    
  }
  else{
    sendInt(outputPath + "2", 1); //en1.Heater On  
  }

  if (Humi1>=70){
    sendInt(outputPath + "15", 1); //en1.Venti On
  }
  else{
    sendInt(outputPath + "15", 0); //en1.Venti Off
  }

  //Enclosure 2
  if (Temp2>=30){
    sendInt(outputPath + "19", 1); //en2.Cooler On
  }
  else if (25<Temp2<30){
    sendInt(outputPath + "19", 0); //en2.Cooler Off
    sendInt(outputPath + "18", 0); //en2.Heater Off     
  }
  else{
    sendInt(outputPath + "18", 1); //en2.Heater On  
  }

  if (Humi2>=70){
    sendInt(outputPath + "5", 1); //en2.Venti On
  }
  else{
    digitalWrite(en2.Venti,LOW);
    sendInt(outputPath + "5", 0); //en2.Venti Off
  }

} //void clim_control

void user_clim_control(control_pins en_number, int control_type){

  switch(control_type){
    case (VentilationOn):
      digitalWrite(en_number.Venti,HIGH); 
      sendInt(outputPath + String(en_number.Venti), 1);
      break;
    case (VentilationOff):
      digitalWrite(en_number.Venti,LOW);
      sendInt(outputPath + String(en_number.Venti), 0);
      break;
    case (CoolerOn):
      digitalWrite(en_number.Cooler,HIGH);
      sendInt(outputPath + String(en_number.Cooler), 1);
      break;
    case (CoolerOff):
      digitalWrite(en_number.Cooler,LOW);
      sendInt(outputPath + String(en_number.Cooler), 0);
      break;
    case (HeaterOn):
      digitalWrite(en_number.Heater,HIGH);
      sendInt(outputPath + String(en_number.Heater), 1);
      break;
    case (HeaterOff):
      digitalWrite(en_number.Heater,LOW);
      sendInt(outputPath + String(en_number.Heater), 0);
      break;
    default:
      break;
  }
} //void user_clim_control

void registry_update(registry* x){  //Registry update function that takes in address of the struct variable
  
  Serial2.print("received");
  while (Serial2.available()==0){
    delay(10); 
  }
  received_msg=Serial2.readString();
  received_msg.trim();
  x->animal_type.push_back(received_msg.toInt());

  Serial2.print("received");
  while (Serial2.available()==0){
    delay(10); 
  }
  received_msg=Serial2.readString();
  received_msg.trim();
  x->ID.push_back(received_msg.toInt());
  Serial2.print("received");

  x->count++;
  
}

//Serial_Com: receives and sorts incoming serial data.
void Serial_Com( void * pvParameters ){
  while(1){
    if (Serial2.available()){
      received_msg=Serial2.readString();
      received_msg.trim();

        if (received_msg=="Register"){
          serial_fetch();
          reg=static_cast<STATE>(received_msg.toInt());
          switch(reg){
            case SalesDelivery:
              registry_update(&salesdelivery);
              break;
            case Recovery:
              registry_update(&recovery);
              break;
            case Deceased:
              registry_update(&deceased);
              break;
            default:
              break;
          }
        }
        if (received_msg=="Control"){
          serial_fetch();
          enclosure=static_cast<ANIMAL>(received_msg.toInt());
          serial_fetch();
          cont=static_cast<CONTROL>(received_msg.toInt());
          if (enclosure==Pig){
            user_clim_control(en1,cont);
            }
        }
          if (enclosure==Chicken){
            user_clim_control(en2,cont);
            
        }
    }
    vTaskDelay(100);
  }
  
}

//Check_Temp, update water and feed level to Cloud every 10 seconds 
void Check_Clim( void * pvParameters ){
  
  while(1){
  // Serial.print("Check_Temp running on core ");
  // Serial.println(xPortGetCoreID());

    //fetch temperature and hum from sensor code;
    Humi1 = dht1.readHumidity();
    Temp1 = dht1.readTemperature();
    Humi2 = dht2.readHumidity();
    Temp2 = dht2.readTemperature();

    if (isnan(Humi1) || isnan(Temp1) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    }

    if (isnan(Humi2) || isnan(Temp2) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    }

    // Read values from potentiometers (resource consumption sensors)
    float ADCwaterLevel = analogRead(Water_sensor);      // Read from pin 32
    float ADCfeedLevel1 = analogRead(Feed_sensor1);      // Read from pin 33
    float ADCfeedLevel2 = analogRead(Feed_sensor2);      // Read from pin 34
    float ADCfeedLevel3 = analogRead(Feed_sensor3);      // Read from pin 35
    float ADCfeedLevel4 = analogRead(Feed_sensor4);      // Read from pin 36
    float ADCfeedLevel5 = analogRead(Feed_sensor5);      // Read from pin 39

    // Convert ADC values to percentage
    float waterLevel = ADCwaterLevel/4100*100;      // Read from pin 32
    float feedLevel1 = ADCfeedLevel1/4100*100;      // Read from pin 33
    float feedLevel2 = ADCfeedLevel2/4100*100;      // Read from pin 34
    float feedLevel3 = ADCfeedLevel3/4100*100;      // Read from pin 35
    float feedLevel4 = ADCfeedLevel4/4100*100;      // Read from pin 36
    float feedLevel5 = ADCfeedLevel5/4100*100;      // Read from pin 39

    // Set potentiometer readings in Firebase
    sendFloat(inputPath + "32", waterLevel);   // Water sensor reading
    sendFloat(inputPath + "33", feedLevel1);   // Feed sensor 1 reading
    sendFloat(inputPath + "34", feedLevel2);   // Feed sensor 2 reading
    sendFloat(inputPath + "35", feedLevel3);   // Feed sensor 3 reading
    sendFloat(inputPath + "36", feedLevel4);   // Feed sensor 4 reading
    sendFloat(inputPath + "39", feedLevel5);   // Feed sensor 5 reading

    //check if the values have changed. (not sure what, xyyx)
    clim_control();

    //OSTimeDlyHMSM(0,0,10,0); 
    tensecondscount += 1;

    vTaskDelay(500);
  } 
}

//Logger_Handlercode update temp and humid every 1 min 
void Logger( void * pvParameters ){ //NOTE to xyyx: instead of using count, can use OSTimeDlyHMSM(0,1,0,0)
   
    while(1){
    // Serial.print("Logger_Handler running on core ");
    // Serial.println(xPortGetCoreID());

    if (tensecondscount >= 6){  //Enters loop when count is equal to 6.

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

        //For serial update via USB to PC
        Serial.print(Temp1);
        Serial.print(",");
        Serial.print(Humi1);
        Serial.print(",");
        Serial.print(Temp2);
        Serial.print(",");
        Serial.println(Humi2);  // End with a newline character

      tensecondscount = 0; //reset at 60 seconds for next data logging
    }

  } //while(1)
  
} //void Logger

//IoT: blinks an LED every 500 ms (xyyx, not sure for what)
void IoT( void * pvParameters ){
   
    while(1){
    // Serial.print("IoT_Handler running on core ");
    // Serial.println(xPortGetCoreID());
    vTaskDelay(500);
    
  }
}


void Resources_Monitor( void * pvParameters ){
   
    while(1){
    // Serial.print("Resources_Monitor_Handler running on core ");
    // Serial.println(xPortGetCoreID());

    vTaskDelay(500);
  }
}


void setup() {
  Serial.begin(115200); 
  Serial2.begin(115200);
  initWiFi();

  esp_task_wdt_init(10, true);

  dht1.begin();
  dht2.begin();

  //IO Initialization
  pinMode(en1.Venti, OUTPUT);
  pinMode(en1.Heater, OUTPUT);
  pinMode(en1.Cooler, OUTPUT);
  pinMode(en2.Venti, OUTPUT);
  pinMode(en2.Heater, OUTPUT);
  pinMode(en2.Cooler, OUTPUT);

  pinMode(Temp_Sensor1, INPUT);
  pinMode(Temp_Sensor2, INPUT);
  pinMode(Feed_sensor1, INPUT);
  pinMode(Feed_sensor2, INPUT);
  pinMode(Feed_sensor3, INPUT);
  pinMode(Feed_sensor4, INPUT);
  pinMode(Feed_sensor5, INPUT);
  pinMode(Water_sensor, INPUT);

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
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
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
  sensorPath1 = databasePath +"/sensor1/";  
  sensorPath2 = databasePath +"/sensor2/";

  // Update database path for inputs
  inputPath = databasePath +"/inputs/";

  // Streaming (whenever data changes on a path)
  // Begin stream on a database path --> UsersData/<user_uid>/outputs
  if (!Firebase.RTDB.beginStream(&stream, listenerPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  // Assign a calback function to run when it detects changes on the database
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  /*------------------------END OF FIREBASE SETUP-----------------------------------*/

        //create a task that will be executed in the Serial_Com() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Serial_Com,   /* Task function. */
                    "Serial Com Manager",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    1,           /* priority of the task */
                    &Serial_Com_Handler,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
  delay(50); 

  //create a task that will be executed in the Check_Temp() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Check_Clim,   /* Task function. */
                    "Check Climate",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    5,           /* priority of the task */
                    &Check_Clim_Handler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Logger() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Logger,   /* Task function. */
                    "Data Logging",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    3,           /* priority of the task */
                    &Logger_Handler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(50); 

     //create a task that will be executed in the IoT() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    IoT,   /* Task function. */
                    "IoT Manager",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    2,           /* priority of the task */
                    &IoT_Handler,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
  delay(50); 

        //create a task that will be executed in the Resources_Monitor() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Resources_Monitor,   /* Task function. */
                    "Resources Monitor",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    4,           /* priority of the task */
                    &Resources_Monitor_Handler,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
  delay(50); 

  
  

}


void loop() {
  
}