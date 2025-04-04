#include <DIYables_Keypad.h>  // DIYables_Keypad library

const int ROW_NUM = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {7, 6, 5, 4}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {11, 10, 9, 8}; //connect to the column pinouts of the keypad

DIYables_Keypad keypad = DIYables_Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

void setup(){
  Serial.begin(9600);
  delay(1000);
  Serial.println("Keypad 4x4 example");
}

void loop(){
  char key = keypad.getKey();

  if (key){
    Serial.println(key);
  }
}
