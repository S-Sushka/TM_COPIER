#include <Wire.h>
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define KEY_PIN 10

#define BUTTON_WRITE_PIN 13
#define BUTTON_READ_PIN  2

OneWire KEY_ONEWIRE(KEY_PIN); 
uint8_t KEY_ID_BUFFER[8];

bool loop_write = false;
bool loop_read = false;


class DISPLAY_CLASS
{
  Adafruit_SSD1306 SSD1306 = Adafruit_SSD1306(128, 64, &Wire, -1);
  unsigned long timeBuffer = millis();


  // >>> Main Menu Animation Help
  bool arrowsPosition = false;

  // >>> Wait Animation Help
  String dots = ".";
  uint8_t counterOfDot = 1;  


  String getID_HIGH_NIBBLE() { return "     "+get_TRUE_HEX(KEY_ID_BUFFER[0])+':'+get_TRUE_HEX(KEY_ID_BUFFER[1])+':'+get_TRUE_HEX(KEY_ID_BUFFER[2])+':'+get_TRUE_HEX(KEY_ID_BUFFER[3])+"     "; }
  String getID_LOW_NIBBLE() { return "     "+get_TRUE_HEX(KEY_ID_BUFFER[4])+':'+get_TRUE_HEX(KEY_ID_BUFFER[5])+':'+get_TRUE_HEX(KEY_ID_BUFFER[6])+':'+get_TRUE_HEX(KEY_ID_BUFFER[7])+"     "; }
  String get_TRUE_HEX(uint8_t number) 
  {
    String result;
        
    if (number < 16)
      result = '0'+String(number, HEX);
    else
      result = String(number, HEX);
    result.toUpperCase();

    return result;
  }

  String repeatChar(char symbol, uint8_t number)
  {
    String result = "";

    for (uint8_t i = 0; i < number; i++)     
      result += symbol;    

    return result;
  }

  
public:    
  enum { ANIMATION_NON = 0, ANIMATION_MAIN_MENU = 1, ANIMATION_WAIT = 2};
  int8_t ANIMATION_NUMBER = ANIMATION_MAIN_MENU;

  void begin() 
  {
    SSD1306.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
    SSD1306.setTextColor(SSD1306_WHITE);
    SSD1306.setTextSize(1); 
  };


  // >>> MENUS
  void showMenu_READING() 
  {
    SET_ANIMATION(ANIMATION_NON);
    
    SSD1306.clearDisplay();
    SSD1306.setCursor(34, 28);
    SSD1306.print("READING...");
    SSD1306.display();          
  }
  void showMenu_WRITING() 
  {
    SET_ANIMATION(ANIMATION_NON);
    
    SSD1306.clearDisplay();
    SSD1306.setCursor(34, 28);
    SSD1306.print("WRITING...");
    SSD1306.display(); 
  }    

  // >>> ANIMATIONS
  void SET_ANIMATION(int8_t NEW_ANIMATION_NUMBER) 
  {    
    ANIMATION_NUMBER = NEW_ANIMATION_NUMBER;   
  }
  
  void UPDATE_ANIMATION() 
  {
    switch(ANIMATION_NUMBER) 
    {
    case ANIMATION_MAIN_MENU: 
      if (millis() - timeBuffer >= 250) 
      {
        arrowsPosition = !arrowsPosition;  
        timeBuffer = millis();
        
        SSD1306.clearDisplay();
      
        SSD1306.setCursor(22, 0);
        SSD1306.print("TM CRUTCH v1.0");
        SSD1306.setCursor(0, 20);

        if (arrowsPosition)
          SSD1306.print("<- READ      WRITE ->");
        else
          SSD1306.print(" <- READ    WRITE -> ");
        
        SSD1306.setCursor(0, 39);
        SSD1306.println("       KEY ID:       ");    
        SSD1306.println(getID_HIGH_NIBBLE());
        SSD1306.print(getID_LOW_NIBBLE());
        
        SSD1306.display();     
      }                
      break;
      
    case ANIMATION_WAIT:
    if (millis() - timeBuffer >= 250) 
    {
      timeBuffer = millis();

      if (counterOfDot <= 3) 
      {        
        SSD1306.clearDisplay();
        
        SSD1306.setCursor(34, 0);
        SSD1306.print("WAIT A KEY");
      
        SSD1306.setTextSize(2);  
        SSD1306.setCursor(46, 10);
        SSD1306.print(repeatChar('.', counterOfDot));
        SSD1306.setTextSize(1);
      
        SSD1306.display();        

        counterOfDot++;  
      }
      else 
        counterOfDot = 1;
    }
    break;         
    }
  }
};
DISPLAY_CLASS SSD1306;


void setup() 
{ 
  Serial.begin(9600);  
  SSD1306.begin();

  pinMode(BUTTON_WRITE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_READ_PIN, INPUT_PULLUP);
}

void loop() 
{
  if (!digitalRead(BUTTON_READ_PIN)) 
    loop_read = true;
  if (!digitalRead(BUTTON_WRITE_PIN)) 
    loop_write = true;

  if (loop_read) 
  {
    if (KEY_READ(KEY_ID_BUFFER)) 
    {
      loop_read = false;           
      SSD1306.SET_ANIMATION(DISPLAY_CLASS::ANIMATION_MAIN_MENU);
    } 
    else     
      SSD1306.SET_ANIMATION(DISPLAY_CLASS::ANIMATION_WAIT);
  }
  
  if (loop_write) 
  {
    if (KEY_WRITE(KEY_ID_BUFFER)) 
    {
      loop_write = false;      
      SSD1306.SET_ANIMATION(DISPLAY_CLASS::ANIMATION_MAIN_MENU);
    } 
    else     
      SSD1306.SET_ANIMATION(DISPLAY_CLASS::ANIMATION_WAIT);        
  }

  SSD1306.UPDATE_ANIMATION();
}


// >>> KEY FUNCTIONS
bool KEY_READ(uint8_t *ID_BUFFER)  // - READ
{    
  bool result = false;

  KEY_ONEWIRE.reset(); 
  delay(50); 

  KEY_ONEWIRE.write(0x33); 
  KEY_ONEWIRE.read_bytes(ID_BUFFER, 8); 
  
  if (ID_BUFFER[0] & ID_BUFFER[1] & ID_BUFFER[2] & ID_BUFFER[3] & ID_BUFFER[4] & ID_BUFFER[5] & ID_BUFFER[6] & ID_BUFFER[7] == 0xFF) 
    return result;
  else  
    SSD1306.showMenu_READING();
  
  return true;
}

bool KEY_WRITE(uint8_t *ID_BUFFER) // - WRITE
{    
  byte data[8]; 
   
  KEY_ONEWIRE.reset(); 
  delay(50); 


  // >>> ПРИЛОЖЕН ЛИ КЛЮЧ?
  KEY_ONEWIRE.write(0x33); 
  KEY_ONEWIRE.read_bytes(data, 8); 
  
  if (data[0] & data[1] & data[2] & data[3] & data[4] & data[5] & data[6] & data[7] == 0xFF) 
    return false;
  else
    SSD1306.showMenu_WRITING(); 

  
  // >>> ВХОДИМ В РЕЖИМ ЗАПИСИ
  KEY_ONEWIRE.skip();
  KEY_ONEWIRE.reset();
  KEY_ONEWIRE.write(0xD1); 
  digitalWrite(KEY_PIN, LOW); 
  pinMode(KEY_PIN, OUTPUT); 
  delayMicroseconds(60);
  pinMode(KEY_PIN, INPUT); 
  digitalWrite(KEY_PIN, HIGH); 
  delay(10);  


  // >>> ЗАПИСЫВАЕМ
  KEY_ONEWIRE.skip();
  KEY_ONEWIRE.reset();
  KEY_ONEWIRE.write(0xD5); // команда записи
  for (byte i=0; i<8; i++) 
  {
    writeByte(ID_BUFFER[i]);
    Serial.print("*");
  }
  Serial.print("\n");  


  // >>> ВЫХОДИМ ИЗ РЕЖИМА ЗАПИСИ
  KEY_ONEWIRE.reset();
  KEY_ONEWIRE.write(0xD1); 
  digitalWrite(KEY_PIN, LOW); 
  pinMode(KEY_PIN, OUTPUT); 
  delayMicroseconds(10);
  pinMode(KEY_PIN, INPUT); 
  digitalWrite(KEY_PIN, HIGH);   
  delay(10);
  
  return true;
}

int writeByte(byte data) {
  int data_bit;
  for(data_bit=0; data_bit<8; data_bit++) {
    if (data & 1) {
      digitalWrite(KEY_PIN, LOW); 
      pinMode(KEY_PIN, OUTPUT);
      delayMicroseconds(60);
      pinMode(KEY_PIN, INPUT); 
      digitalWrite(KEY_PIN, HIGH);
      delay(10);  
    } else {
      digitalWrite(KEY_PIN, LOW); 
      pinMode(KEY_PIN, OUTPUT);
      pinMode(KEY_PIN, INPUT); 
      digitalWrite(KEY_PIN, HIGH);
      delay(10);  
    }
    data = data >> 1;
  }
  return 0;
}
