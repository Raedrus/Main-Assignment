#include <Arduino.h>
#include <list>
// include the library code:
#include <LiquidCrystal.h>



// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

/*Variables from ESP1*/
int Temp1=1; //Temperature for enclosure 1
int Humi1=10; //Humidity for enclosure 1
int Temp2=9; //Temperature for enclosure 2
int Humi2=10; //Humidity for enclosure 2

/*Variables to ESP1*/
enum STATE {SalesDelivery,Recovery,Deceased};
STATE reg;

enum CONTROL {Ventilation, Cooler, Heater, OFF, ON};
CONTROL cont;

int ID;

/*Input*/
String key = ""; 



/*--------------Functions--------------*/
void Post(){
    lcd.clear();
    lcd.print("");
}

void Registration(STATE *r, CONTROL *cont){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Registration of");
    lcd.setCursor(0, 1);
    lcd.print("Animals");
    delay(1000);

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
  lcd.begin(16, 2);
  // Print a message to the LCD, indicate the LCD is working
  lcd.print("LCD is powered on");
  delay(1000);

}
 
void loop() {
  LCD_Temp();
  delay(1);
  
  if (key=="y")
    Registration(&reg , &cont);

}
