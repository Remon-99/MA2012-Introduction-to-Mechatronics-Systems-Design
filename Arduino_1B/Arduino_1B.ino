// include libraries required:
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
// stepper motor stuff
#include "UCN5804.h"

// initialise all pins on LCD
const int rs = 6, en = 7, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// initialise Tx and Rx pins
#define Tx_pin 13
#define Rx_pin 12

// 1.8 degree per step, 200 steps per revolution
#define motorSteps 200     // change this depending on the number of steps
                           // per revolution of your motor

// define the pins that the motor is attached to. You can use
// any digital I/O pins.
#define dirPin   11
#define stepPin  10
#define halfPin  9
#define phasePin 8

UCN5804 steering(motorSteps, dirPin, stepPin, halfPin, phasePin);

SoftwareSerial LCD_slave(Rx_pin, Tx_pin);

int num1;
int num2 = 0;
int curr_steering_angle = 150;
void setup() {
  // put your setup code here, to run once:
  pinMode(Rx_pin, INPUT);
  pinMode(Tx_pin, OUTPUT);

  lcd.begin(16, 2);
  LCD_slave.begin(9600);
  Serial.begin(9600);

  // initialise stepper
  steering.setSpeed(60);
}

int print_forward_reverse(int dir){
  lcd.setCursor(0, 0);
  if (dir == 202){
    lcd.print("Forward");
  } else if (dir == 203){
    lcd.print("Reverse");
  } else if (dir == 204){
    lcd.print("Neutral");
  }
  return dir;
}

int print_duty_cycle(int duty){
  if ((duty >= 100) || (duty < 10)){
    lcd.setCursor(9, 0);
    lcd.print("     ");
    if (duty > 100){
      return duty;
    }
  }
  if ((duty != num2) && (duty < 100)){
    lcd.setCursor(9, 0);
    lcd.print("     ");
  }
  lcd.setCursor(9, 0);
  lcd.print(duty);
  return duty;
}

int print_seatbelt(int seatbelt){
  lcd.setCursor(0, 1);
  if (seatbelt == 211){
    lcd.print("Safety belt!");
  } else if (seatbelt == 212) {
    lcd.print("            ");
  }
  return seatbelt;
}

int steer_stepper(int incoming_num, int curr_steering_num){
//  unsigned long mapping_constant = 33 / 48;
  float steps_to_move_float;
  int steps_to_move_int;
  Serial.println(incoming_num);
  Serial.println(curr_steering_num);
  if ((curr_steering_num > 148) && curr_steering_num < 154){
    if ((incoming_num > 148) && (incoming_num < 154)){
      // do nothing since the pot2 is within range
      return incoming_num;
    }
  }
  steps_to_move_float = ((float) incoming_num -  (float) curr_steering_num) * (33.0/48.0);
  steps_to_move_int = (int) steps_to_move_float;
  Serial.println(steps_to_move_float);
  steering.step(steps_to_move_int);
  return incoming_num;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (LCD_slave.available()){
    num1 = LCD_slave.read();
//    if ((num2 == 0) || (num2 != num1)){
//      num2 = num1;
//    }
//    Serial.println(num2);
    if (num1 > 210){
      num2 = print_seatbelt(num1);
    } else if (num1 > 201){
      num2 = print_forward_reverse(num1);
    }
    else if (num1 > 100){
      curr_steering_angle = steer_stepper(num1, curr_steering_angle);
    } else {
      num2 = print_duty_cycle(num1);
    }
  }

}
