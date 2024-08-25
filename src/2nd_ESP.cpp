#include <Arduino.h>
#include <list>
// include the library code:
#include <cmath>
#include <LiquidCrystal.h>
#include <Keypad.h>

/*-------------LCD Initialization-------------*/
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
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
//Waiting for Serial Response
void send_wait(String event);
//Send serial to ESP1
void SerialCom();
//check serial communication input
void Check_serial();
//Clear and Display String at LCD for 1s
void Post(String event);
//Registration of animals 
void Registration();
//Control the system
void Control_sys();
//Display Temp and Humid at LCD
void LCD_Temp();

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(LCD_COLS, LCD_ROWS);
  Serial.begin(115200);
  // Print a message to the LCD, indicate the LCD is working
  lcd.print("LCD is powered on");
  delay(1000);
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
    
    Post("Back To Main...");
    LCD_Temp();

}

//Waiting for Serial Response
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
        send_wait(String(cont));
    }

}

//check serial communication input
void Check_serial(){
    received_data="";
    
    //Check if any data is sent to ESP2
    if (Serial.available()==1){
        received_data=Serial.readString();
    }

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

//Clear and Display String at LCD for 1s
void Post(String event){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(event); 
    delay(1000);
}

void Control_sys(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Control");
    delay(1000);
    lcd.clear();

    //Select option
    i=0;
    int j=0;
    //Category Input
    while(key!='A'){
        lcd.clear();
        key='*';
        switch (i)
        {
        case '0':
            lcd.setCursor(0, 0);
            lcd.print("1:Ventilation");
            lcd.setCursor(0, 1);
            lcd.print("2:Next    A:Exit");
            break;
        case '1':
            lcd.setCursor(0, 0);
            lcd.print("1:Cooler");
            lcd.setCursor(0, 1);
            lcd.print("2:Next    A:Exit");
            break;
        case '2':
            lcd.setCursor(0, 0);
            lcd.print("1:Heater");
            lcd.setCursor(0, 1);
            lcd.print("2:Next    A:Exit");
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
            Post("Process Failed");
            key='A';
            break;
        }
    }

    while(key!='A'){
        lcd.clear();
        key='*';
        
        lcd.setCursor(0, 0);
        lcd.print("1:On");
        lcd.setCursor(0, 1);
        lcd.print("2:Off     A:Exit");
              
        while (key!='1' &&key!='2'  &&key!='A' ){
            delay(1);
            key=kpd.getKey();
        }

        if (key=='1'){
            //j is even, meaning ON
            break;
        }

        else if (key=='2'){
            j++; //odd is off, originally is even
            break;
        }
            
        else if (key=='A') 
            break;
        
        else{
            Post("Process Failed");
            key='A';
            break;
        }
    }

    if (key!='A')
    {
        cont=static_cast<CONTROL>(j);
        com=Controlsys;
        SerialCom();
    }
    
    
}

//Registration of animals 
void Registration(){
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
        key='*';
        switch (i)
        {
        case '0':
            lcd.setCursor(0, 0);
            lcd.print("1:Sales/Delivery");
            lcd.setCursor(0, 1);
            lcd.print("2:Next    A:Exit");
            break;
        case '1':
            lcd.setCursor(0, 0);
            lcd.print("1:Recovery");
            lcd.setCursor(0, 1);
            lcd.print("2:Next    A:Exit");
            break;
        case '2':
            lcd.setCursor(0, 0);
            lcd.print("1:Deceased");
            lcd.setCursor(0, 1);
            lcd.print("2:Next    A:Exit");
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
            Post("Process Failed");
            key='A';
            break;
        }
    }

    lcd.clear();
    
    //Animal Type Input
    if (key!='A'){
        lcd.setCursor(0, 0);
        lcd.print("1:Pig    A:EXIT");
        lcd.setCursor(0, 1);
        lcd.print("2:Chicken");
        key='*';

        while (key!='1' &&key!='2'  &&key!='A' ){
            delay(1);
            key=kpd.getKey(); 
            }

        lcd.clear();
        switch (key)
        {
        case '1':
            ani=Pig;
            key='*';
            break;
        case '2':
            ani=Chicken;
            key='*';
            break;
        case 'A':
            Post("Exiting...");
            break;
        default:
            Post("Process Failed");
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

        key='*';
        key=kpd.getKey();

        if (key=='A' || key=='C')
            break;

        else if (int(key)>=0 && int(key)<=9){
            ID+=int(key)*pow(10,i);
            i++;
            delay(200);
        }  
    }

    if(key=='C'){
        com=Update;
        SerialCom();
        Post("Registering...");
    }

    key='*';
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
