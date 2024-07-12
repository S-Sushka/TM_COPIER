#include<OneWire.h>

#define pin 10
OneWire iButton(pin);
uint8_t keyBuffer[8] = {0x14, 0x88, 0, 0, 0, 0, 0, 0};

uint8_t serialBuffer = 0;


void setup() 
{  
  Serial.begin(9600);
}

void loop() 
{
  if (Serial.available()) 
  {
    serialBuffer = Serial.read();

    if (serialBuffer == 'R')
    {
      Serial.println(">>> READ");
      Serial.println("WAIT KEY...");
      while(!KEY_READ(keyBuffer));

      Serial.print("KEY CODE: ");
      for (int i = 0; i < 8; i++) 
      {
        Serial.print(keyBuffer[i], HEX);
        if (i < 7)
          Serial.print(':');
      }
      Serial.println("");

      iButton.skip();
      iButton.reset();            
    }
    else if (serialBuffer == 'W') 
    {
      Serial.println(">>> WRITE");
      Serial.println("WAIT KEY...");  

      while(!KEY_WRITE(keyBuffer));
      
      iButton.skip();
      iButton.reset();      
    }
  }
}


// --------------------------------- READ
bool KEY_READ(uint8_t *ID_BUFFER) 
{  
  bool result = false;

  iButton.reset(); 
  delay(50); 

  iButton.write(0x33); 
  iButton.read_bytes(ID_BUFFER, 8); 
  
  if (ID_BUFFER[0] & ID_BUFFER[1] & ID_BUFFER[2] & ID_BUFFER[3] & ID_BUFFER[4] & ID_BUFFER[5] & ID_BUFFER[6] & ID_BUFFER[7] == 0xFF) 
    return result; 
  
  return true;
}

// --------------------------------- WRITE
bool KEY_WRITE(uint8_t *ID_BUFFER) 
{ 
  bool result = false;
  byte data[8]; 
   
  iButton.reset(); 
  delay(50); 


  // >>> ПРИЛОЖЕН ЛИ КЛЮЧ?
  iButton.write(0x33); 
  iButton.read_bytes(data, 8); 
  
  if (data[0] & data[1] & data[2] & data[3] & data[4] & data[5] & data[6] & data[7] == 0xFF) 
    return result; 

  
  // >>> ВХОДИМ В РЕЖИМ ЗАПИСИ
  iButton.skip();
  iButton.reset();
  iButton.write(0xD1); 
  digitalWrite(pin, LOW); 
  pinMode(pin, OUTPUT); 
  delayMicroseconds(60);
  pinMode(pin, INPUT); 
  digitalWrite(pin, HIGH); 
  delay(10);  


  // >>> ЗАПИСЫВАЕМ
  iButton.skip();
  iButton.reset();
  iButton.write(0xD5); // команда записи
  for (byte i=0; i<8; i++) 
  {
    writeByte(ID_BUFFER[i]);
    Serial.print("*");
  }
  Serial.print("\n");  


  // >>> ВЫХОДИМ ИЗ РЕЖИМА ЗАПИСИ
  iButton.reset();
  iButton.write(0xD1); 
  digitalWrite(pin, LOW); 
  pinMode(pin, OUTPUT); 
  delayMicroseconds(10);
  pinMode(pin, INPUT); 
  digitalWrite(pin, HIGH);   
  delay(10);
  return true;
}

int writeByte(byte data) {
  int data_bit;
  for(data_bit=0; data_bit<8; data_bit++) {
    if (data & 1) {
      digitalWrite(pin, LOW); 
      pinMode(pin, OUTPUT);
      delayMicroseconds(60);
      pinMode(pin, INPUT); 
      digitalWrite(pin, HIGH);
      delay(10);  
    } else {
      digitalWrite(pin, LOW); 
      pinMode(pin, OUTPUT);
      pinMode(pin, INPUT); 
      digitalWrite(pin, HIGH);
      delay(10);  
    }
    data = data >> 1;
  }
  return 0;
}
