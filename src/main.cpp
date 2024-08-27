#include <Arduino.h>
#include <FreeRTOSConfig.h>
#include <vector>

using namespace std;

TaskHandle_t Serial_Com_Handler;  //Manages incoming serial data    
TaskHandle_t Check_Clim_Handler;  // Check Temp + Climate Control
TaskHandle_t Logger_Handler;      // Log Data 
TaskHandle_t IoT_Handler;         // Check and Update for IoT 
TaskHandle_t Resources_Monitor_Handler;    // Check Feed (5) + Water Consumption

// TaskHandle_t Keypad_Check_Handler;      // Check Keypad_Check and the Input

/*-----------------PLS CHANGE ACCORDING TO YOUR NEEDS------------------*/
// IO pins
/*INPUT*/
/*Temperature And Humidity Sensor*/
const int Temp_Sensor1 = 4;    //DHT11
const int Temp_Sensor2 = 12;   //DHT11

/*Resources Consumption Sensor*/
const int Water_sensor = 12;   //Potentiometer
const int Feed_sensor1 = 4;    //
const int Feed_sensor2 = 12;   //
const int Feed_sensor3 = 12;   //
const int Feed_sensor4 = 12;   //
const int Feed_sensor5 = 12;   //

    
/*OUTPUT*/
/*Temperature And Humidity*/
const int Venti1 = 4;           //YelLED
const int Heater1 = 12;           //RedLED
const int Cooler1 = 4;            //BlueLED
const int Venti2 = 4;           //YelLED
const int Heater2 = 12;           //RedLED
const int Cooler2 = 4;            //BlueLED

/*SERIAL COMMUNICATION*/
const int rx_pin = 16;       //RX pin
const int tx_pin = 17;       //TX pin

/*Variables*/
int Temp1=1; //Temperature for enclosure 1
int Humi1=10; //Humidity for enclosure 1
int Temp2=9; //Temperature for enclosure 2
int Humi2=10; //Humidity for enclosure 2
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
}enclosure1,enclosure2,prev_enclosure1,prev_enclosure2;



///////////////////////////////////////////////////////////////
/* * * * * * * * * * Tasks and Functions * * * * * * * * * * */

void LCD_Serial(){
  //Serial Com to 2nd ESP for LCD output
}

void serial_fetch(){
  Serial.print("received");
          
  while (Serial.available()==0){
    delay(10);
  }
  received_msg=Serial.readString();
  received_msg.trim();
}

bool check_clim_ifchange(){
  if (prev_enclosure1.temp.back()!=enclosure1.temp.back()
  ||prev_enclosure1.hum.back()!=enclosure1.hum.back()
  ||prev_enclosure2.temp.back()!=enclosure2.temp.back()
  ||prev_enclosure2.hum.back()!=enclosure2.hum.back()){
    return true;
  }
  else{
    return false;
  }
}

void clim_control(){
  //Enclosure 1
  if (Temp1>=30){
    digitalWrite(Cooler1,HIGH);
  }
  else if (25<Temp1<30){
    digitalWrite(Cooler1,LOW);
    digitalWrite(Heater1,LOW);
  }
  else{
    digitalWrite(Heater1,HIGH);
  }

  if (Humi1>=70){
    digitalWrite(Venti1,HIGH);
  }
  else{
    digitalWrite(Venti1,LOW);
  }

  //Enclosure 2
  if (Temp2>=30){
    digitalWrite(Cooler2,HIGH);
  }
  else if (25<Temp2<30){
    digitalWrite(Cooler2,LOW);
    digitalWrite(Heater2,LOW);
  }
  else{
    digitalWrite(Heater2,HIGH);
  }

  if (Humi2>=70){
    digitalWrite(Venti2,HIGH);
  }
  else{
    digitalWrite(Venti2,LOW);
  }
}
void registry_update(registry* x){  //Registry update function that takes in address of the struct variable
  
  Serial.print("received");
  while (Serial.available()==0){
    delay(10); 
  }
  received_msg=Serial.readString();
  received_msg.trim();
  x->animal_type.push_back(received_msg.toInt());

  Serial.print("received");
  while (Serial.available()==0){
    delay(10); 
  }
  received_msg=Serial.readString();
  received_msg.trim();
  x->ID.push_back(received_msg.toInt());
  Serial.print("received");

  x->count++;
}


//Serial_Com: receives and sorts incoming serial data.
void Serial_Com( void * pvParameters ){
  
  while(1){
    if (Serial.available()){
      received_msg=Serial.readString();
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
          if (enclosure==Pig){
            serial_fetch();
            switch(cont){
              case (VentilationOn):
                digitalWrite(Venti1,HIGH);
                break;
              case (VentilationOff):
                digitalWrite(Venti1,LOW);
                break;
              case (CoolerOn):
                digitalWrite(Cooler1,HIGH);
                break;
              case (CoolerOff):
                digitalWrite(Cooler1,LOW);
                break;
              case (HeaterOn):
                digitalWrite(Heater1,HIGH);
                break;
              case (HeaterOff):
                digitalWrite(Heater1,LOW);
                break;
              default:
                break;
            }
          }
          if (enclosure==Chicken){
            serial_fetch();
            switch(cont){
              case (VentilationOn):
                digitalWrite(Venti2,HIGH);
                break;
              case (VentilationOff):
                digitalWrite(Venti2,LOW);
                break;
              case (CoolerOn):
                digitalWrite(Cooler2,HIGH);
                break;
              case (CoolerOff):
                digitalWrite(Cooler2,LOW);
                break;
              case (HeaterOn):
                digitalWrite(Heater2,HIGH);
                break;
              case (HeaterOff):
                digitalWrite(Heater2,LOW);
                break;
              default:
                break;
            }
          }
        }
    }
  }
}  

//Check_Temp: blinks an LED every 1s
void Check_Clim( void * pvParameters ){
  
  while(1){
  Serial.print("Check_Temp running on core ");
  Serial.println(xPortGetCoreID());


    //fetch temperature and hum from sensor code;


    //check if the values have changed.
  if (check_clim_ifchange()){

  }
    
  } 
}

//Logger_Handlercode: blinks an LED every 500 ms
void Logger( void * pvParameters ){
   
    while(1){
    Serial.print("Logger_Handler running on core ");
    Serial.println(xPortGetCoreID());

    
  }
}

//IoT: blinks an LED every 500 ms
void IoT( void * pvParameters ){
   
    while(1){
    Serial.print("IoT_Handler running on core ");
    Serial.println(xPortGetCoreID());

    
  }
}


void Resources_Monitor( void * pvParameters ){
   
    while(1){
    Serial.print("Resources_Monitor_Handler running on core ");
    Serial.println(xPortGetCoreID());

    
  }
}



void setup() {
  Serial.begin(115200); 
  
  //IO Initialization
  pinMode(Venti1, OUTPUT);
  pinMode(Heater1, OUTPUT);
  pinMode(Cooler1, OUTPUT);
  pinMode(Venti2, OUTPUT);
  pinMode(Heater2, OUTPUT);
  pinMode(Cooler2, OUTPUT);

  pinMode(Temp_Sensor1, INPUT);
  pinMode(Temp_Sensor2, INPUT);
  pinMode(Feed_sensor1, INPUT);
  pinMode(Feed_sensor2, INPUT);
  pinMode(Feed_sensor3, INPUT);
  pinMode(Feed_sensor4, INPUT);
  pinMode(Feed_sensor5, INPUT);
  pinMode(Water_sensor, INPUT);


        //create a task that will be executed in the Serial_Com() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Serial_Com,   /* Task function. */
                    "Serial Com Manager",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    1,           /* priority of the task */
                    &Serial_Com_Handler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

  //create a task that will be executed in the Check_Temp() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Check_Clim,   /* Task function. */
                    "Check CLimate",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Check_Clim_Handler,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Logger() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Logger,   /* Task function. */
                    "Data Logging",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    1,           /* priority of the task */
                    &Logger_Handler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

     //create a task that will be executed in the IoT() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    IoT,   /* Task function. */
                    "IoT Manager",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    1,           /* priority of the task */
                    &IoT_Handler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

        //create a task that will be executed in the Resources_Monitor() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Resources_Monitor,   /* Task function. */
                    "Resources Monitor",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    1,           /* priority of the task */
                    &Resources_Monitor_Handler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

  
  

}





void loop() {
  
}