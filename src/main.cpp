#include <Arduino.h>
#include <list>
// include the library code:
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

enum CONTROL {Ventilation, Cooler, Heater, OFF, ON};
CONTROL cont;

int ID ={};

/*Input*/
char16_t key; 



/*--------------Functions--------------*/
void SerialCom(){

}

//Clear and Display String for 1s
void Post(String event){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(event); 
    delay(1000);
}

void Registration(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Registration of");
    lcd.setCursor(0, 1);
    lcd.print("Animals");
    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    key='*';
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
        break;
    }

    int i=0;
    if (key=='*'){
        while(key!='A'){
            if (i==0){
                lcd.setCursor(0, 0);
                lcd.print("1:Sales/Delivery");
                lcd.setCursor(0, 1);
                lcd.print("2:Next   A:Exit");
            }
            
            else if (i==1)
            {
                /* code */
            }

            else if (i==2)
            {
                /* code */
            }
            
            
            key=kpd.getKey();
        }
    }


}

void LCD_Temp(){
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Temp : %.2d", Temp1, "  ", "%.2d", Temp2);
  lcd.setCursor(0, 1);
  lcd.printf("Humid: %.2d", Humi1, "  ", "%.2d", Humi2); 
}

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(LCD_COLS, LCD_ROWS);
  // Print a message to the LCD, indicate the LCD is working
  lcd.print("LCD is powered on");
  delay(1000);

}
 
void loop() {
  LCD_Temp();
  ;
  delay(1);

  if (kpd.getKey()=='*')
    Registration();
    Post("Back To Main...");

}
