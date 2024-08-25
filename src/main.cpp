#include <Arduino.h>
#include <FreeRTOSConfig.h>


TaskHandle_t climateControlHandler;
TaskHandle_t loggerHandler;
TaskHandle_t IOTHandler;
TaskHandle_t resourceMonitorHandler;
TaskHandle_t keypadHandler;

void logger(void * pvParameters);
void climateControl(void * pvParameters);
void IOT(void * pvParameters);
void resourceMonitor(void * pvParameters);
void keypad(void * pvParameters);
void LCD();

// LED pins
const int RedLED = 2;
const int YelLED = 4;

void setup() {
  Serial.begin(115200); 
  pinMode(RedLED, OUTPUT);
  pinMode(YelLED, OUTPUT);

  

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    climateControl,   /* Task function. */
                    "Climate Control",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    2,           /* priority of the task */
                    &climateControlHandler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

  xTaskCreatePinnedToCore(
                    logger,   /* Task function. */
                    "Data logger",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        // parameter of the task 
                    2,           /* priority of the task */
                    &loggerHandler,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 

  xTaskCreatePinnedToCore(
                  IOT,   /* Task function. */
                  "IOT Handler",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &IOTHandler,      /* Task handle to keep track of created task */
                  0);          /* pin task to core 0 */                  
  delay(500); 

  xTaskCreatePinnedToCore(
                  resourceMonitor,   /* Task function. */
                  "Resource Monitor",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &resourceMonitorHandler,      /* Task handle to keep track of created task */
                  0);          /* pin task to core 0 */                  
  delay(500); 

  xTaskCreatePinnedToCore(
                  keypad,   /* Task function. */
                  "Keypad",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &keypadHandler,      /* Task handle to keep track of created task */
                  0);          /* pin task to core 0 */                  
  delay(500); 
}

//create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  

//Task1code: blinks an LED every 1s
void checkTemp( void * pvParameters ){
  
    while(1){
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());

    digitalWrite(RedLED, HIGH);
    delay(1000);
    digitalWrite(RedLED, LOW);
    delay(1000);
  } 
}

//Task2code: blinks an LED every 500 ms
void logger( void * pvParameters ){
   
    while(1){
    Serial.print("Task2 running on core ");
    Serial.println(xPortGetCoreID());

    digitalWrite(YelLED, HIGH);
    delay(500);
    digitalWrite(YelLED, LOW);
    delay(500);
  }

}
void IOT( void * pvParameters ){

}

void resourceMonitor( void * pvParameters ){

}

void keypad( void * pvParameters ){
//saves the input and menu choices, including the id and number of animals. Saved data will be processed at logger.
}

void LCD(){

}
void loop() {
  
}


