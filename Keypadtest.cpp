#include <Arduino.h>
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 21, 22,23}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {2, 4, 5, 18}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
unsigned long t_hold;
char holdKey;
String msg;


void setup() {
    Serial.begin(9600);
    loopCount = 0;
    startTime = millis();
    msg = "";
}


void loop() {
    loopCount++;
    // if ( (millis()-startTime)>5000 ) {
    //     Serial.print("Average loops per second = ");
    //     Serial.println(loopCount/5);
    //     startTime = millis();
    //     loopCount = 0;
    // }

    char key=kpd.getKey();
    
    if (key){
    holdKey = key;
    Serial.println(key);
   }
 
   if (kpd.getState() == HOLD) {
      if ((millis() - t_hold) > 100 ) {
          switch (holdKey) {
              case '#':
                Serial.println("# held");
                break;
              default:
                break;
          }
          t_hold = millis();
      }
   }
}
    // if (key) {
    //     if (key == '#') {
            
    //         if (kpd.getState() == HOLD) {
    //             // When the # key is being held
    //             if ((millis() - t_hold) > 100) {
    //                 Serial.print("Key: ");
    //                 Serial.print(key);
    //                 Serial.println(" is held.");  
    //             }
    //             t_hold=millis();
    //         } 
    //         // else if (kpd.getState() == RELEASED) {
                
    //         //     // Handle quick press without hold
    //         //     Serial.print("Key: ");
    //         //     Serial.print(key);
    //         //     Serial.println(" is released before hold time.");
    //         // }
            
            
            
    //     }
     
    //     else{
    //         truekey=key;
    //         Serial.print("Key: ");
    //         Serial.print(truekey);
    //         Serial.println(" is pressed.");
    //         t_hold = 0;
            
    //     }
    // } 
        
    


    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    // if (kpd.getKeys())
    // {
    //     for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
    //     {
    //         if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
    //         {
    //             switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
    //                 case PRESSED:
    //                 msg = " PRESSED.";
    //             break;
    //                 case HOLD:
    //                 msg = " HOLD.";
    //             break;
    //                 case RELEASED:
    //                 msg = " RELEASED.";
    //             break;
    //                 case IDLE:
    //                 msg = " IDLE.";
    //             }
    //             Serial.print("Key ");
    //             Serial.print(kpd.key[i].kchar);
    //             Serial.println(msg);
    //         }
    //     }
    // }
  // End loop