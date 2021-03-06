#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

//Pinout
#define signal 2
#define OUTA 3
#define OUTB 14
#define BTN 4
#define buzzer 5
#define led1 6
#define led2 7
#define TX_BT 8
#define RX_BT 9
#define button1 10
#define button2 11
#define button3 12
#define button4 14
#define button5 15
#define button6 16
#define debounce_delay 200
#define LCD_REFRECH 10

//Instances
LiquidCrystal_I2C lcd(0x27,16,2);
SoftwareSerial BLE(RX_BT, TX_BT);

//Variables
bool state, prev_state;
bool currentDir;
int currentStateCLK;
int lastStateCLK;
int analog_read;
volatile unsigned long counter = 0;
volatile long counterRotary = 0;
float distance = 0.0, rapport = 988.0;
bool FWD_BWD = 0;
bool ETALON = 0;
bool GO_EXIT = 0;

//Functions
void updateCounter();
float get_distance();
int buttonPressed();
void mainMenu();
void etalonnage();
void go();


void setup() {
  pinMode(button1, INPUT);pinMode(button2, INPUT);pinMode(button3, INPUT);pinMode(button4, INPUT);
  pinMode(button5, INPUT);pinMode(button6, INPUT);pinMode(led1, OUTPUT);pinMode(buzzer, OUTPUT);
  pinMode(buzzer, OUTPUT); pinMode(OUTA, INPUT); pinMode(OUTB, INPUT); pinMode(signal, INPUT);
  attachInterrupt(digitalPinToInterrupt(OUTA), interruptRotary, CHANGE);
  attachInterrupt(digitalPinToInterrupt(signal), interruptSignal, CHANGE);
  lastStateCLK = digitalRead(OUTA);
  BLE.begin(9600);
  BLE.println("CLUB CELEC *** CTTP");
  lcd.init(); lcd.backlight(); lcd.noBlink(); lcd.noCursor();
  lcd.home(); lcd.print("***CLUB CELEC ***");
  lcd.setCursor(0,1); lcd.print("******CTTP******");
  delay(3000);
  lcd.clear();
}

void loop() {
  // si le bouton de l'etalonnage est appuyer executer la fonction
  // d'etalonnage sinon acceder au menu principale
  if(ETALON == 1) etalonnage();
  else mainMenu();
}

void interruptRotary() {
  currentStateCLK = digitalRead(OUTA);
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
		if (digitalRead(OUTB) != currentStateCLK) {
			counterRotary --;
			currentDir =true;
		} 
    else {
			counterRotary ++;
			currentDir =false;
		}
  }
}

void interruptSignal() {
  if (FWD_BWD) {
    counter++;
  } else {
    counter--;
  } 
  
}

/*
Fonction pour avoir la distance
eq: dist = counter * 100m / 988
*/
float get_distance() {
  return (counter * 100) / rapport;
}

/*
Fonction pour detecter le bouton appuyer
*/
int buttonPressed(){
  if(digitalRead(button1)){
    tone(buzzer, 35, 20);
    delay(debounce_delay);
    return 1;
  }
  else if (digitalRead(button2)){
    tone(buzzer, 46, 20);
    delay(debounce_delay);
    return 2;
  }
  else if (digitalRead(button3)){
    tone(buzzer, 62, 20);
    delay(debounce_delay);
    return 3;
  }
  else if (digitalRead(button4)){
    tone(buzzer, 82, 20);
    delay(debounce_delay);
    return 4;
  }
  else if (digitalRead(button5)){
    tone(buzzer, 110, 20);
    delay(debounce_delay);
    return 5;
  }
  else if (digitalRead(button6)){
    tone(buzzer, 147, 20);
    delay(debounce_delay);
    return 6;
  }
  return 0;
}

void mainMenu(){
  lcd.clear(); 
  lcd.home();
  lcd.print("Distance:");
  lcd.setCursor(9,0);lcd.print(get_distance(), 1);lcd.setCursor(15,0);lcd.print("m");
  lcd.setCursor(0,1);
  lcd.print("A:CL B:Av C:Arr");
  //BLE.print("Distance: ");BLE.println(get_distance());
  //delay(LCD_REFRECH);
  switch (buttonPressed()){
  case 1:
    counter = 0;
    lcd.clear();
    break;
  case 2:
    FWD_BWD = 0;
    break;
  case 3:
    FWD_BWD = 1;
    break;
  case 4:
    ETALON = 1;
    break;
  case 5:
    rapport = 988;
    break;
  default:
    break;
  }
}

void etalonnage(){
  lcd.noBlink(); lcd.noCursor(); lcd.clear(); lcd.home();
  lcd.print("***Etalonnage***");
  lcd.setCursor(0,1);
  lcd.print("A:Go  B:OK  C:Re");
  delay(LCD_REFRECH);
  lcd.noBlink(); lcd.noCursor(); lcd.clear(); lcd.home();
  switch (buttonPressed()){
  case 1: //Bouton Go appuyer
    go();
    break;
  case 2: //Bouton OK appuyer
    rapport = (rapport + counter) / 2;
    lcd.setCursor(5,1); lcd.cursor(); lcd.blink();
    delay(1000);
    //sauvegarder rapport sur EEPROM
    break;
  case 3:
    counter = 0;
    lcd.setCursor(11,1); lcd.cursor(); lcd.blink();
    delay(1000);
    break;
  case 4:
    ETALON = 0;
    break;
  default:
    break;
  }
}

void go(){
  tone(buzzer, 147, 20); tone(buzzer, 147, 20);
  counter = 0;
  ETALON = 0;
  GO_EXIT = 0;
  while (1){
    lcd.home();
    lcd.print("Etln100m:");
    lcd.setCursor(9,0);lcd.print(get_distance(), 1);lcd.setCursor(15,0);lcd.print("m");
    lcd.setCursor(0,1); lcd.cursor(); lcd.blink();
    lcd.print("A:GO  B:OK  C:Re");
    delay(LCD_REFRECH);

    switch (buttonPressed())
    {
    case 2:
      rapport = (rapport + counter) / 2;
      GO_EXIT = 1;
      lcd.setCursor(6,1); lcd.cursor(); lcd.blink();
      delay(1000);
      break;
    case 3:
      counter = 0;
      lcd.setCursor(12,1); lcd.cursor(); lcd.blink();
      delay(1000);
      break;
    case 4:
      ETALON = 0;
      GO_EXIT = 1;
      break;
    default:
      break;
    }
    
    if(GO_EXIT) break;
  }
}
