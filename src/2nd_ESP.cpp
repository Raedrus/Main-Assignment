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
byte rowPins[KP_ROWS] = {19, 21, 22,23}; //connect to the row pinouts of the kpd
byte colPins[KP_COLS] = {2, 4, 5, 18}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins,KP_ROWS, KP_COLS );


/*Variables from ESP1*/
int Temp1=1; //Temperature for enclosure 1
int Humi1=10; //Humidity for enclosure 1
int Temp2=9; //Temperature for enclosure 2
int Humi2=10; //Humidity for enclosure 2

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
void Check_serial();
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
  // Print a message to the LCD, indicate the LCD is working
  Display.Show1s("LCD is powered on");
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

}

//Send + Waiting for Serial Response
void send_wait(String event){
    Serial.print(event);
    while (Serial.available()==0){
            delay(1);
            //waiting response
        }
    //read the response msg
    received_data=Serial.readString();    
}

//Send serial to ESP1
void SerialCom(){
    if (com==0)//Update
    {
        send_wait("Register");
        
        /*UPDATE the registration info*/
        send_wait(String(reg));
        send_wait(String(ani));
        send_wait(String(ID));
    }

    else if (com==1)//Controlsys
    {
        send_wait("Control");
        send_wait(String(enclosure));
        send_wait(String(cont));
    }

}

//check serial communication input
void Check_serial(){
    received_data="";
    
    //Check if any data is sent to ESP2
    if (Serial.available()==1){
        received_data=Serial.readString();
        received_data.trim();

        //Update Temperature and Humidity
        if (received_data=="Temp"){
            send_wait("OK");
            Temp1=received_data.toInt();
            send_wait("OK");
            Humi1=received_data.toInt();
            send_wait("OK");
            Temp2=received_data.toInt();
            send_wait("OK");
            Humi2=received_data.toInt();
            
            //Update the data at LCD
            LCD_Temp();
        }   
    }

    
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
        key=kpd.getKey();

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
        lcd.clear();
        key='Z';
        switch (i)
        {
        case '0':
            Display.Selection("Ventilation");
            break;
        case '1':
            Display.Selection("Cooler");
            break;
        case '2':
            Display.Selection("Heater");
            break;
        default:
            i=0;
            break;
        }

        while (key!='1' &&key!='2'  &&key!='A' ){
            delay(1);
            key=kpd.getKey();
        }

        if (key=='1'){
            j=i*2;
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

    while(key!='A'){
        lcd.clear();
        key='Z';
        
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
            
        else if (key=='A') 
            break;
        
        else{
            Display.Show1s("Process Failed");
            key='A';
            break;
        }
    }

    if (key!='A') // send the instruction to ESP1
    {
        cont=static_cast<CONTROL>(j);
        com=Controlsys;
        SerialCom();
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
            lcd.clear();
            key='Z';
            switch (i)
            {
            case '0':
                Display.Selection("Sales/Delivery");
                break;
            case '1':
                Display.Selection("Recovery");
                break;
            case '2':
                Display.Selection("Deceased");
                break;
            default:
                i=0;
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
        
        
        i=0;
        ID=0;
        //Animal ID input
        while(key!='A'){
            lcd.setCursor(0, 0);
            lcd.printf("Animal ID: %.3",ID);
            lcd.setCursor(0, 1);
            lcd.print("C:Confirm A:Exit");

            key='Z';
            key=kpd.getKey();

            if (key=='A' || key=='C')
                break;

            else if (int(key)>=0 && int(key)<=9){
                ID=int(key)+ID*pow(10,i);
                i++;
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
  lcd.setCursor(0, 0);
  lcd.printf("Temp : %.2d", Temp1, "  ", "%.2d", Temp2);
  lcd.setCursor(0, 1);
  lcd.printf("Humid: %.2d", Humi1, "  ", "%.2d", Humi2); 
}

