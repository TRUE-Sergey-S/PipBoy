#include <SPI.h>
//#include <nRF24L01.h>
//#include <RF24.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24  radio(9, 10);
#include <LiquidCrystal_I2C.h>
#include "Keypad.h"

const byte Rows = 4;
const byte Cols = 3;
char keymap[Rows][Cols] =
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte cPins[Cols] = {4, 2, 6};
byte rPins[Rows] = {3, 8, 7, 5};
int sendNumber;
Keypad kpd = Keypad(makeKeymap(keymap), rPins, cPins, Rows, Cols);
LiquidCrystal_I2C lcd(0x27, 20, 4);
byte pipe;
int changePosition = 0;
int itemName[6];//1 = ArtBox //2 - HeatDetektor //3 - LaserDetektor //9 - test
char* itemNameChar[] = {"ArtBox", "HeatDetektor", "LaserDetektor", "No item", "No item", "test item"};
int data[3];

uint8_t tochki[8] = {B0, B00000, B0, B0, B0, B0, B10101};
uint8_t bukva_P[8] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t bukva_Ya[8] = {B01111, B10001, B10001, B01111, B00101, B01001, B10001};
uint8_t bukva_L[8] = {0x3, 0x7, 0x5, 0x5, 0xD, 0x9, 0x19};
uint8_t bukva_Lm[8] = {0, 0, B01111, B00101, B00101, B10101, B01001};
uint8_t bukva_Mz[8] = {0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E};
uint8_t bukva_I[8] = {0x11, 0x13, 0x13, 0x15, 0x19, 0x19, 0x11};
uint8_t bukva_D[8] = {B01111, B00101, B00101, B01001, B10001, B11111, 0x11};
uint8_t bukva_G[8] = {B11111, B10001, B10000, B10000, B10000, B10000, B10000};
uint8_t bukva_IY[8] = {B01110, B00000, B10001, B10011, B10101, B11001, B10001};
uint8_t bukva_Z[8] = {B01110, B10001, B00001, B00010, B00001, B10001, B01110};
uint8_t bukva_ZH[8] = {B10101, B10101, B10101, B11111, B10101, B10101, B10101};
uint8_t bukva_Y[8] = {B10001, B10001, B10001, B01010, B00100, B01000, B10000};
uint8_t bukva_B[8] = {B11110, B10000, B10000, B11110, B10001, B10001, B11110};
uint8_t bukva_CH[8] = {B10001, B10001, B10001, B01111, B00001, B00001, B00001};
uint8_t bukva_IYI[8] = {B10001, B10001, B10001, B11001, B10101, B10101, B11001};
uint8_t bukva_TS[8] = {B10010, B10010, B10010, B10010, B10010, B10010, B11111, B00001};
uint8_t bukva_Shch[8] = {B10101, B10101, B10101, B10101, B10101, B10101, B11111, B00001}; // Буква "Щ"
uint8_t bukva_Sh[8]  = {B10101, B10101, B10101, B10101, B10101, B10101, B11111, B00000,}; // Буква "Ш"
uint8_t bukva_full[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
uint8_t bukva_nfull[8] = {B11111, B10001, B10001, B10001, B10001, B10001, B10001, B11111};

void setup() {
  Serial.begin(9600);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  lcd.init();
  lcd.backlight();

  delay(100);
  radio.begin();                             // Инициируем работу nRF24L01+
  radio.setChannel(5);                       // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_250KBPS);      // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS). При скорости 2 Мб/с, задействуются сразу два канала (выбранный и следующий за ним). (самая дальнобойная 250KBPS, но скорость меньше)
  radio.setPALevel      (RF24_PA_HIGH);      // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openReadingPipe (0, 0xAABBCCDD11LL); // Открываем 1 трубу с идентификатором 0xAABBCCDD11 для приема данных ("приёмник"-на одном канале может быть открыто до 6 разных труб), (которые должны отличаться только последним байтом идентификатора)
  radio.openReadingPipe (1, 0xAABBCCDD22LL);
  radio.openReadingPipe (2, 0xAABBCCDD33LL);
  radio.openReadingPipe (3, 0xAABBCCDD44LL);
  radio.openReadingPipe (4, 0xAABBCCDD55LL);
  radio.openReadingPipe (5, 0xAABBCCDD66LL);

  //radio.openWritingPipe (0xAABBCCDD12LL);    // отправка данных на контейнер с артефактами
  radio.startListening  ();                  // Включаем приемник, начинаем прослушивать открытую трубу
  delay(100);
  changePositionPrintMesage();
  Serial.println("запущен");
}

void PrintOnDispleyMesage(int printWalue, int printDevise) {
  switch (printDevise) {
    case 1:
      if (printWalue <= 99) {// нужно настроить получение пакетов!!!!!!
        //анимация открытия контейнера 1-99
        lcd.createChar(0, bukva_full); // не менять
        lcd.createChar(1, bukva_I); // не менять
        lcd.createChar(2, bukva_IYI); // не менять
        lcd.createChar(3, bukva_CH);
        lcd.createChar(4, bukva_D);
        lcd.createChar(5, bukva_TS);
      }
      break;
    case 2:
      for (int i = 0; i <= 10; i++) {
        if (printWalue == 600) {
          lcd.createChar(0, bukva_IY);
          lcd.createChar(1, bukva_IYI);
          lcd.createChar(2, bukva_D);
          lcd.createChar(5, bukva_I);
          lcd.createChar(4, bukva_ZH);
          lcd.createChar(3, bukva_B);
          lcd.setCursor(0, 0);
          lcd.print("                    ");
          lcd.setCursor(0, 1);
          lcd.print("                    ");
          lcd.setCursor(0, 0);
          lcd.print("*BH");
          lcd.write(5);
          lcd.print("MAH");
          lcd.write(5);
          lcd.print("E  ");
          lcd.write(2);
          lcd.print("B");
          lcd.write(5);
          lcd.write(4);
          lcd.print("EH");
          lcd.write(5);
          lcd.print("E*");
          lcd.setCursor(0, 1);
          lcd.print("*****O");
          lcd.write(3);
          lcd.print("HAPY");
          lcd.write(4);
          lcd.write(5);
          lcd.print("HO*****");
          delay(100);
        }
      }
      if (printWalue == 500) {
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(0, 1);
        lcd.print("                    ");
      }
      break;
    case 6:
      if (printWalue == 611) {
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 0);
        lcd.print("LightON");
      }
      if (printWalue == 622) {
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(0, 1);
        lcd.print("                    ");
        lcd.setCursor(0, 0);
        lcd.print("LightOFF");
      }
      break;
  }
  changePositionPrintMesage();
}

void TimePrint() {
  int time = millis() / 1000;
  if (time / 60 / 60 < 10) {
    Serial.print ("0");
  }
  Serial.print (time / 60 / 60);
  Serial.print (":");
  if (time / 60 % 60 < 10) {
    Serial.print ("0");
  }
  Serial.print ((time / 60) % 60);
  Serial.print (":");
  if (time % 60 < 10) {
    Serial.print ("0");
  }
  Serial.println (time % 60);
}
int lastWaluePipe0 = 0;
int lastWaluePipe1 = 0;
int lastWaluePipe2 = 0;
int lastWaluePipe3 = 0;
int lastWaluePipe4 = 0;
int lastWaluePipe5 = 0;

void sendMesageArtBox(int sendNumber) {
  int tryCount = 0;
  while (tryCount < 100) {
    delay (50);
    radio.stopListening();
    delay (50);
    switch (changePosition) {
      case 0:
        radio.openWritingPipe (0xAABBCCDD12LL);
        break;
      case 1:
        radio.openWritingPipe (0xAABBCCDD2ALL);
        break;
      case 2:
        //radio.openWritingPipe (0xAABBCCDD3ALL);
        break;
      case 3:
        //radio.openWritingPipe (0xAABBCCDD4ALL);
        break;
      case 4:
        //radio.openWritingPipe (0xAABBCCDD5ALL);
        break;
      case 5:
        radio.openWritingPipe (0xAABBCCDD6ALL);
        break;

    }
    delay (50);
    if (radio.write(&sendNumber, sizeof(sendNumber))) {
      Serial.println("Отправка прошла успешно, значение: ");
      Serial.print(sendNumber);
      Serial.println("                                                ");
      break;
    } else {
      Serial.println("Ошибка отправки, значение: ");
      Serial.print(sendNumber);
    }
    Serial.println("                         ");
    tryCount++;
  }
  delay (50);
  radio.startListening();
  delay (50);
}

void PerformActions(int thisPipe, int type, int value) {
  lcd.createChar(6, bukva_full);
  lcd.createChar(7, bukva_nfull);
  lcd.setCursor(thisPipe, 3);
  lcd.write(6);
  itemName[thisPipe] = type;
  switch (type) {
    case 1:// ArtBox
      if (lastWaluePipe0 != value) {
        delay(20);
        lastWaluePipe0 = value;
        Serial.println("                        ");
        Serial.println("++++++++++++++++++++++++");
        TimePrint();
        Serial.print("ArtBox value = ");
        Serial.println(value);
        Serial.println("++++++++++++++++++++++++");
        PrintOnDispleyMesage(value, type);
      }
      break;
    case 2://HeatDetektor
      if (value == 600 || lastWaluePipe1 != value) {
        delay(20);
        lastWaluePipe1 = value;
        Serial.println("                        ");
        Serial.println("++++++++++++++++++++++++");
        TimePrint();
        Serial.print("HeatDetektor value = ");
        Serial.println(value);
        Serial.println("++++++++++++++++++++++++");
        PrintOnDispleyMesage(value, type);
      }
      break;
    case 3:
      //LaserDetektor
      break;
    case 4:
      break;
    case 5:
      break;
    case 6://test
      if (lastWaluePipe5 != value) {
        delay(20);
        lastWaluePipe5 = value;
        Serial.println("                        ");
        Serial.println("++++++++++++++++++++++++");
        TimePrint();
        Serial.print("HeatDetektor value = ");
        Serial.println(value);
        Serial.println("++++++++++++++++++++++++");
        PrintOnDispleyMesage(value, type);
      }
      break;
  }
}

void ChangePosition(int bottonValue) {
  switch (bottonValue) {
    case 1://Left
      if (changePosition <= 0) {
        changePosition = 5;
        break;
      }
      changePosition--;
      break;
    case 2://right
      if (changePosition >= 5) {
        changePosition = 0;
        break;
      }
      changePosition++;
      break;
  }
  changePositionPrintMesage();
}
void changePositionPrintMesage() {
  //itemName[3];//1 = ArtBox //2 - HeatDetektor //3 - LaserDetektor //6 - Test
  lcd.setCursor(0, 2);
  lcd.print("                  ");
  lcd.setCursor(changePosition, 2);
  lcd.print("V-:");
  lcd.print(itemNameChar[changePosition]);
}

void PrintSendingNumber()
{

  lcd.setCursor(8, 3);
  lcd.print("VALUE: ");
  lcd.print("    ");
  lcd.setCursor(15, 3);
  lcd.print(sendNumber);
}

unsigned long offSlotTimer[7];
void loop() {
  Serial.println("цыкл");
  char keypressed = kpd.getKey();
  int keyToInt;

  if (keypressed != NO_KEY)
  {
    Serial.println(keypressed);
    keyToInt = keypressed - 48;
    if (keyToInt >= 0)
    {
      if (sendNumber < 1)
      {
        sendNumber = keyToInt;
      } else if (sendNumber >= 1 & sendNumber < 10)
      {
        sendNumber = sendNumber * 10;
        sendNumber = sendNumber + keyToInt;
      } else if (sendNumber >= 10 & sendNumber < 100)
      {
        sendNumber = sendNumber * 10;
        sendNumber = sendNumber + keyToInt;
      }
    } else
    {
      if (keypressed == '*') {
        sendNumber = 0;
        Serial.println("Clean");
      } else if (keypressed == '#') {
        if (sendNumber > 0) {
          Serial.print("Sending number ");
          Serial.println(sendNumber);
          sendMesageArtBox(sendNumber);
          lcd.setCursor(15, 3);
          lcd.print("Send");
          delay(600);
          sendNumber = 0;
          Serial.println("Clean");
        }
      }
    }
    PrintSendingNumber();
  }


  /*
    if (digitalRead(bottonOn) == 1) {
      Serial.println("*********bottonOn pressed*****************");
      sendMesageArtBox(666);
    }

    if (digitalRead(bottonOff) == 1) {
      Serial.println("*********bottonOff pressed*****************");
      sendMesageArtBox(777);
    }
  */

    if (digitalRead(A2) == 1) {
    Serial.println("*********bottonLeft pressed*****************");
    ChangePosition(1);
    }

    if (digitalRead(A1) == 1) {
    Serial.println("*********bottonRight pressed*****************");
    ChangePosition(2);
    }
  delay(100);
  
  if (radio.available(&pipe)) {              // Если в буфере имеются принятые данные, то получаем номер трубы, по которой они пришли, по ссылке на переменную pipe
    int radioInput;
    radio.read(&data, sizeof(data));         // Читаем данные в переменную TEST и указываем сколько байт читать
    Serial.println("                        ");
      Serial.println("++++++++++++++++++++++++");
      Serial.println(data[0]);
      Serial.println(data[1]);
      Serial.println(data[2]);
      Serial.println("++++++++++++++++++++++++");
    lcd.createChar(7, bukva_nfull);
    offSlotTimer[6] = millis();
    delay(20);
    if (pipe == 0) { //ArtBox
      PerformActions(pipe, data[0], data[1]);
      offSlotTimer[0] = millis();
    } else {
      if (millis() - offSlotTimer[0] > 10000) {
        offSlotTimer[0] = millis();
        lcd.setCursor(0, 3);
        lcd.write(7);
      }
    }
    if (pipe == 1) { //heatDetektor
      PerformActions(pipe, data[0], data[1]);
      offSlotTimer[1] = millis();
    } else {
      if (millis() - offSlotTimer[1] > 3000) {
        offSlotTimer[1] = millis();
        lcd.setCursor(1, 3);
        lcd.write(7);
      }
    }
    if (pipe == 2) { //
      PerformActions(pipe, data[0], data[1]);
      offSlotTimer[2] = millis();
    } else {
      if (millis() - offSlotTimer[2] > 1000) {
        offSlotTimer[2] = millis();
        lcd.setCursor(2, 3);
        lcd.write(7);
      }
    }
    if (pipe == 3) { //
      PerformActions(pipe, data[0], data[1]);
    } else {
      lcd.setCursor(3, 3);
      lcd.write(7);
    }
    if (pipe == 4) { //
      PerformActions(pipe, data[0], data[1]);
    } else {
      lcd.setCursor(4, 3);
      lcd.write(7);
    }
    if (pipe == 5) { //test item
      PerformActions(pipe, data[0], data[1]);
    } else {
      lcd.setCursor(9, 3);
      lcd.write(7);
    }
  } else {
    if (millis() - offSlotTimer[6] > 5000) {
      offSlotTimer[6] = millis();
      for (int i = 0; i <= 5; i++) {
        lcd.setCursor(i, 3);
        lcd.write(7);
      }
    }
  }
}
