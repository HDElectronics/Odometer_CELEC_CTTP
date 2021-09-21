#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

//Pinout
#define button1 4
#define button2 5
#define button3 6
#define button4 7
#define button5 10
#define button6 11
#define led1 12
#define buzzer 13
#define TX_BT 8
#define RX_BT 9
#define debounce_delay 200

//Instances
LiquidCrystal_I2C lcd(0x27,16,2);
SoftwareSerial sSerial(RX_BT, TX_BT);

//Variables
bool state, prev_state;
int analog_read;
unsigned long counter = 0;
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


void setup() {
  pinMode(button1, INPUT);pinMode(button2, INPUT);pinMode(button3, INPUT);pinMode(button4, INPUT);
  pinMode(button5, INPUT);pinMode(button6, INPUT);pinMode(led1, OUTPUT);pinMode(buzzer, OUTPUT);
  lcd.init(); lcd.backlight(); lcd.noBlink(); lcd.noCursor();
  lcd.home(); lcd.print("//*CLUB CELEC \\\\*");
  lcd.setCursor(0,1); lcd.print("*****CTTP*****");
  delay(3000);
  lcd.clear();
}

void loop() {
  // si le bouton de l'etalonnage est appuyer executer la fonction
  // d'etalonnage sinon acceder au menu principale
  if(ETALON == 1) etalonnage();
  mainMenu();
}

/*
Pour marche avant
Lire capteur, definir etat du signal (High ou Low), si il y a un 
chagement d'etat incrementer la variable counter
*/
void updateCounterFWD(){
  analog_read = analogRead(A0);
  if(analog_read < 50) state = 0;
  else if(analog_read > 500) state = 1;
  if(state != prev_state) counter++;
  prev_state = state;
}

/*
Pour marche avant
Lire capteur, definir etat du signal (High ou Low), si il y a un 
chagement d'etat decrementer la variable counter, sans depasser 0
*/
void updateCounterBWD(){
  analog_read = analogRead(A0);
  if(analog_read < 50) state = 0;
  else if(analog_read > 500) state = 1;
  if(state != prev_state && counter > 0) counter--;
  prev_state = state;
}

/*
Fonction pour avoir la distance
eq: dist = counter * 100m / 988
*/
float get_distanceFWD(){
  if(FWD_BWD == 0) updateCounterFWD();
  else updateCounterBWD();
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
  lcd.clear(); lcd.home();
  lcd.print("Distance: ");
  lcd.setCursor(10,0);lcd.print(get_distanceFWD());lcd.setCursor(15,0);lcd.print("m");
  lcd.setCursor(0,1);
  lcd.print("A:CL B:Av C:Arr");
  if(FWD_BWD){lcd.setCursor(10,1); lcd.cursor(); lcd.blink();}
  else {lcd.setCursor(5,1); lcd.cursor(); lcd.blink();}
  delay(1);
  switch (buttonPressed()){
  case 1:
    counter = 0;
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
  lcd.print("/**Etalonnage**\\");
  lcd.setCursor(0,1);
  lcd.print("A:Go  B:OK  C:Re");
  delay(1);
  switch (buttonPressed()){
  case 1: //Bouton Go appuyer
    go();
    break;
  case 2: //Bouton OK appuyer
    rapport = (rapport + counter) / 2;
    //sauvegarder rapport sur EEPROM
    break;
  case 3:
    counter = 0;
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
    lcd.clear(); lcd.home();
    lcd.print("Etalonnage:");
    lcd.setCursor(10,0);lcd.print(get_distanceFWD());lcd.setCursor(15,0);lcd.print("m");
    lcd.setCursor(0,1); lcd.cursor(); lcd.blink();
    lcd.print("A:GO  B:OK  C:Re");

    switch (buttonPressed())
    {
    case 2:
      rapport = (rapport + counter) / 2;
      GO_EXIT = 1;
      break;
    case 3:
      counter = 0;
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
