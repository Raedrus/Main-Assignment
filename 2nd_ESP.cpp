#include <Arduino.h>
#include <list>
// include the library code:
#include <cmath>
#include <LiquidCrystal.h>
#include <Keypad.h>



/*-------------LCD Initialization-------------*/
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(14, 26, 27, 32, 33, 25); //RS, EN, D4, D5, D6, and D7
const byte LCD_ROWS = 2; //four rows
const byte LCD_COLS = 16; //three columns

/*-------------KEYPAD Initialization-------------*/
const byte KP_ROWS = 4; //four rows
const byte KP_COLS = 4; //three columns
char keys[KP_ROWS][KP_COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte rowPins[KP_ROWS] = {18, 5, 4, 15}; //connect to the row pinouts of the kpd
byte colPins[KP_COLS] = {23, 22, 21, 19}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins,KP_ROWS, KP_COLS );


/*Variables from ESP1*/
int Temp1; //Temperature for enclosure 1
int Humi1; //Humidity for enclosure 1
int Temp2; //Temperature for enclosure 2
int Humi2; //Humidity for enclosure 2

/*Variables to ESP1*/
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
char16_t key; //keypad input 
int i=0;      //universal
String received_data;

enum Communication {Update, Controlsys};
Communication com;

/*--------------Functions--------------*/
//Send + Waiting for Serial Response
void send_wait(String event);
//Send serial to ESP1
void SerialCom();
//check serial communication input
bool Check_serial();
//Registration of animals 
void Registration();
//Control the system
void Control_sys();
//Display Temp and Humid at LCD
void LCD_Temp();

//Class for Display at LCD: 
class Post
{
private:
    /* data */
public:
    //Display String at LCD for 1s
    void Show1s(String Event){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(Event); 
        delay(1000);
    }
    //For selection purpose
    void Selection(String Event){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("1:");
        lcd.print(Event);
        lcd.setCursor(0, 1);
        lcd.print("2:Next    A:Exit");
    }
};
Post Display;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(LCD_COLS, LCD_ROWS);
  Serial.begin(115200);
  Serial2.begin(115200);
  // Print a message to the LCD, indicate the LCD is working
  Display.Show1s("Waiting for ESP1");
//   while(1){
//     if (Serial2.available()>1){
//         received_data=Serial2.readString();
//         received_data.trim();
//         Display.Show1s(received_data);
//     }
//     if (kpd.getKey()=='#'){
//             break;
//         }
//   }
  while(!Check_serial())
    {
        if (kpd.getKey()=='#'){
            break;
        }
    }
  LCD_Temp();
}
 
void loop() {
  Check_serial();
  if (kpd.getKey()=='#'){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1:Registration");
    lcd.setCursor(0, 1);
    lcd.print("2:Control");
    while (key!='1' &&key!='2' ){
            delay(1);
            key=kpd.getKey();
        }
    
    switch (key)
    {
    case '1':
        Registration();
        break;
    case '2':
        Control_sys();
        break;
    default:
        break;
    }
    Display.Show1s("Back To Main...");
    LCD_Temp();
    key='Z';
  }

}

//Send + Waiting for Serial Response
void send_wait(String event){
    Serial2.flush();
    int c;
    Serial2.print(event);
    Serial.print(event);
    while (Serial2.available()==0){
            delay(1);
            c++;
            if (c>100){
                // Serial2.print(event);
                // Serial.print(event);
                c=0;
            }
            //waiting response
        }
    //read the response msg
    
    received_data=Serial2.readString();
    Serial.println("REceivED");
    // received_data.trim();  
    Serial.println(received_data);  
}

//Send serial to ESP1
void SerialCom(){
    Check_serial();
    if (com==Update)//Update
    {
        send_wait("Register");
        
        /*UPDATE the registration info*/
        send_wait(String(reg));
        send_wait(String(ani));
        send_wait(String(ID));
    }

    else if (com==Controlsys)//Controlsys
    {   
        
        
        send_wait("Control");

        send_wait(String(enclosure));
        send_wait(String(cont));
    }
}

//check serial communication input
bool Check_serial(){
    // received_data="";
    
    //Check if any data is sent to ESP2
    if (Serial2.available()>1){
        received_data=Serial2.readString();
        received_data.trim();
        Serial.print(received_data);
        //Update Temperature and Humidity
        if (received_data=="Clim"){
            send_wait("OKTemp");
            Temp1=received_data.toInt();
            send_wait("OKTemp");
            Humi1=received_data.toInt();
            send_wait("OKTemp");
            Temp2=received_data.toInt();
            send_wait("OKTemp");
            Humi2=received_data.toInt();
            Serial2.print("OKTemp");

            Serial.println(Temp1);
            Serial.println(Humi1);
            Serial.println(Temp2);
            Serial.println(Humi2);
            LCD_Temp();
        }   

        return true;
    }

    else
        return false;
}


void Control_sys(){
    Display.Show1s("System Control");
       
    //Choose enclosure
    while(key!='A'){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.printf("Enclosure");
        lcd.setCursor(0, 1);
        lcd.print("1:Pig  2:Chicken");

        key='Z';
        while (key!='1' &&key!='2'  &&key!='A' ){
            delay(1);
            key=kpd.getKey();
        }

        if (key=='1'){
            enclosure=Pig;
            break;
        }

        else if (key=='2'){
            enclosure=Chicken;
            break;
        }       
        
    }
    
    //Select option
    i=0;
    int j=0;
    //Category Input
    while(key!='A'){
        delay(1.5);
        key='Z';
        switch (i)
        {
        case 1:
            Display.Selection("Cooler");
            break;
        case 2:
            Display.Selection("Heater");
            break;
        default:
            i=0;
            Display.Selection("Ventilation");
            break;
        }

        while (key!='1' &&key!='2'  &&key!='A' ){
            delay(1);
            key=kpd.getKey();
        }

        if (key=='1'){
            j=i*2;
            break;
        }

        else if (key=='2'){
            i++;
        }
            
        else if (key=='A') 
            break;
        
        else{
            Display.Show1s("Process Failed");
            key='A';
            break;
        }
    }

    while(key!='A'){
        key='Z';
        delay(1.5);
        lcd.clear();
        
        
        lcd.setCursor(0, 0);
        lcd.print("1:On");
        lcd.setCursor(0, 1);
        lcd.print("2:Off     A:Exit");
              
        while (key!='1' &&key!='2'  &&key!='A' ){
            delay(1);
            key=kpd.getKey();
        }

        if (key=='1'){
            //when j is even, meaning ON
            break;
        }

        else if (key=='2'){
            j++; //j odd is off, originally is even
            break;
        }
            
        else if (key=='A') {
            break;
        }
            
        
        else{
            Display.Show1s("Process Failed");
            key='A';
            break;
        }
    }

    if (key!='A') // send the instruction to ESP1
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Sending Command"); 
        cont=static_cast<CONTROL>(j);
        com=Controlsys;
        SerialCom();
        Display.Show1s("Done");  
    }
    
    
}

//Registration of animals 
void Registration(){
    while(key!='A'){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Registration of");
        lcd.setCursor(0, 1);
        lcd.print("Animals");
        delay(1000);
        lcd.clear();

        i=0;
        //Category Input
        while(key!='A'){
            
            key='Z';
            switch (i)
            {
            case 1:
                Display.Selection("Recovery");
                break;
            case 2:
                Display.Selection("Deceased");
                break;
            default:
                i=0;
                Display.Selection("Sales/Delivery");
                break;
            }

            while (key!='1' &&key!='2'  &&key!='A' ){
                delay(1);
                key=kpd.getKey();
            }

            if (key=='1'){
                reg=static_cast<STATE>(i);
                delay(200);
                break;
            }

            else if (key=='2'){
                i++;
                delay(200);
            }
                
            else if (key=='A') 
                break;
            
            else{
                Display.Show1s("Process Failed");
                key='A';
                break;
            }
        }

        lcd.clear();
        
        //Animal Type Input
        if (key!='A'){
            key='Z';
            lcd.setCursor(0, 0);
            lcd.print("1:Pig    A:EXIT");
            lcd.setCursor(0, 1);
            lcd.print("2:Chicken");
            

            while (key!='1' &&key!='2'  &&key!='A' ){
                delay(1);
                key=kpd.getKey(); 
                }

            lcd.clear();
            switch (key)
            {
            case '1':
                ani=Pig;
                key='Z';
                break;
            case '2':
                ani=Chicken;
                key='Z';
                break;
            case 'A':
                Display.Show1s("Exiting...");
                break;
            default:
                Display.Show1s("Process Failed");
                key='A';
                break;
            }
        }
        

        ID=0;
        //Animal ID input
        while(key!='A'){
            lcd.setCursor(0, 0);
            lcd.printf("Animal ID: ");
            lcd.printf("%.3d",ID);
            lcd.setCursor(0, 1);
            lcd.print("C:Confirm A:Exit");

            key='Z';
            key=kpd.getKey();

            if (key=='A' || key=='C')
                break;

            //Change from ASCII to Normal Int
            else if (int(key)>=48 && int(key)<=57){
                ID=int(key)-48+ID*10;
                delay(200);
            }  
        }

        if(key=='C'){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Registering...");
            com=Update; //Set to Update config
            SerialCom(); //Send data to the ESP1
            Display.Show1s("Done");
        }
    }
       
    key='Z';
}

//Display Temp and Humid at LCD
void LCD_Temp(){
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  delay(3);
  lcd.setCursor(0, 0);
  lcd.printf("Temp :  %02d", Temp1);
  lcd.printf("  %02d", Temp2);
  lcd.setCursor(0, 1);
  lcd.printf("Humid:  %02d", Humi1);
  lcd.printf("  %02d", Humi2); 
}
