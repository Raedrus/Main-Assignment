#include <Arduino.h>
#include <FreeRTOSConfig.h>

TaskHandle_t Serial_Com_Handler;  //Manages incoming serial data    
TaskHandle_t Check_Temp_Handler;  // Check Temp + Climate Control
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
const int Venti = 4;           //YelLED
const int Heat = 12;           //RedLED
const int Cool = 4;            //BlueLED


/*SERIAL COMMUNICATION*/
const int rx_pin = 16;       //RX pin
const int tx_pin = 17;       //TX pin

/*Variables*/
int Temp1=1; //Temperature for enclosure 1
int Humi1=10; //Humidity for enclosure 1
int Temp2=9; //Temperature for enclosure 2
int Humi2=10; //Humidity for enclosure 2

/*Enumerations to match data coming from ESP2*/
enum ANIMAL {Pig, Chicken};
ANIMAL ani;

enum STATE {SalesDelivery,Recovery,Deceased};
STATE reg;

int ID;

enum CONTROL {VentilationOn, VentilationOff, CoolerOn, CoolerOff, 
HeaterOn, HeaterOff};
CONTROL cont;

/*Internal Variables*/
String received_msg;



///////////////////////////////////////////////////////////////
/* * * * * * * * * * Tasks and Functions * * * * * * * * * * */

void LCD_Serial(){
  //Serial Com to 2nd ESP for LCD output
}


//Serial_Com: receives and sorts incoming serial data.
void Serial_Com( void * pvParameters ){
  
  while(1){
    if (Serial.available()==0){
      delay(1);
      received_msg=Serial.readString();
      received_msg.trim();

      if (received_msg=="Register"){

      }
    }
    
  
  
  
  
  }
  
}  
  

//Check_Temp: blinks an LED every 1s
void Check_Temp( void * pvParameters ){
  
    while(1){
    Serial.print("Check_Temp running on core ");
    Serial.println(xPortGetCoreID());



    
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
  pinMode(Venti, OUTPUT);
  pinMode(Heat, OUTPUT);
  pinMode(Cool, OUTPUT);

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
                    Check_Temp,   /* Task function. */
                    "Check Temperature",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Check_Temp_Handler,      /* Task handle to keep track of created task */
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