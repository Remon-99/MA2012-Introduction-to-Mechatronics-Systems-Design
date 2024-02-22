#include <SoftwareSerial.h>

// define all pins needed
#define buzzer_pin 2
#define forward_pin 3
#define reverse_pin 4
#define dc_pin_1 5
#define dc_pin_2 6
#define red_led 7
#define green_led 8
#define opto_pin 10

#define pot1_pin A0
#define pot2_pin A1
#define push_pin A5

// communication pins
#define Tx_pin 12
#define Rx_pin 13

SoftwareSerial LCD_slave(Rx_pin, Tx_pin);

char forward_flag;
bool start_flag, seatbelt_flag;
int ref_time, ref_time_2, curr_time, ref_throttle, throttle, ref_duty_cycle, duty_cycle;
int steer_angle, pot2_val;

void setup() {
  // put your setup code here, to run once:
  // output pins
  pinMode(buzzer_pin, OUTPUT);
  pinMode(dc_pin_1, OUTPUT);
  pinMode(dc_pin_2, OUTPUT);

  // led pins
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);

  // tx pin
  pinMode(Tx_pin, OUTPUT);

  // input pins  
  pinMode(forward_pin, INPUT);
  pinMode(reverse_pin, INPUT);
  pinMode(opto_pin, INPUT);
  pinMode(pot1_pin, INPUT);
  pinMode(pot2_pin, INPUT);
  pinMode(push_pin, INPUT);
  pinMode(Rx_pin, INPUT);

  Serial.begin(9600);
  LCD_slave.begin(9600); 

  forward_flag = forward_reverse_neutral();
  throttle = analogRead(pot1_pin) / 4;
  steer_angle = ((float) analogRead(pot2_pin)/1023) * 100;
  duty_cycle = ((float) throttle / 255) * 100;
  seatbelt_flag = check_seatbelt(2);        
  write_dir(forward_flag);
  delay(100);
  LCD_slave.write(duty_cycle);
  start_flag = 0;
  digitalWrite(red_led, HIGH);
}

// function for forward and reverse
char forward_reverse_neutral(){
  // read the forward pin first
  if (digitalRead(forward_pin) == LOW){
    // forward
    return 'f';
  } else if (digitalRead(reverse_pin) == LOW){
    // reverse
    return 'r';
  } else {
    // neutral
    return 'n';
  }
}

// function to control the direction
void direction (char dir, int throttle)
{
  //dir f = forward
  //    r = reverse
  //    n = stop

  switch (dir)
  {
    case ('r'):
    {
      analogWrite(dc_pin_1, throttle);
      digitalWrite(dc_pin_2,LOW);
      break;
    }
    case('f'):
    {
      analogWrite(dc_pin_1, 255 - throttle);
      digitalWrite(dc_pin_2,HIGH);
      break;
    }
    case('n'):
    {
      analogWrite(dc_pin_1, 0);
      digitalWrite(dc_pin_2,LOW);
      break;
    }
    default: // also brake
     {
      analogWrite(dc_pin_1, 0);
      digitalWrite(dc_pin_2,LOW);
      break;
    }
  } 
}

void blink_led(){
  int interval = 250;
  int curr_millis = millis();

  static int ref_millis;
  if (ref_time == 0){
    ref_time = millis();
  }
  if ((curr_millis - ref_time) > (2 * interval)){
    digitalWrite(red_led, HIGH);
    ref_time = 0;    
  } else if ((curr_millis - ref_time) > interval){
    digitalWrite(red_led, LOW);
  }
}

void sound_buzzer(){
  int interval = 250;
  int curr_millis = millis();

  static int ref_millis;
  if (forward_flag == 'r'){
    if (ref_time_2 == 0){
      ref_time_2 = millis();
  }
    if ((curr_millis - ref_time_2) > (2 * interval)){
      tone(buzzer_pin, 500);
      ref_time_2 = 0;
    } else if ((curr_millis - ref_time_2) > interval){
      noTone(buzzer_pin);
    }   
  }
 
}

void write_dir(char dir){
  if (dir == 'f'){
    LCD_slave.write(202);
  } else if (dir == 'r'){
    LCD_slave.write(203);
  } else if (dir == 'n'){
    LCD_slave.write(204);
  }
}

char check_dir(char init_dir){
  char curr_dir = forward_reverse_neutral();
  if (init_dir != curr_dir){
    write_dir(curr_dir);
    return curr_dir;
  } else {
    return init_dir;
  }
}

int check_duty_cycle(int init_duty){
  throttle = analogRead(pot1_pin) / 4;
  int curr_duty = ((float) throttle / 255) * 100;
  if (abs(curr_duty - init_duty) >= 5){
    LCD_slave.write(curr_duty);
    return curr_duty;
  } else {
    return init_duty;
  }
}

int check_steering(int init_steer){
  pot2_val = analogRead(pot2_pin); 
  int curr_angle = ((float) pot2_val / 1023) * 100;
  if (abs(curr_angle - init_steer) >= 1){
    Serial.println(curr_angle);
    LCD_slave.write(curr_angle + 101);
    return curr_angle;
  } else {
    return init_steer;
  }
}

void write_seatbelt(int curr_seatbelt){
  if (curr_seatbelt == 1){
    LCD_slave.write(211);
  } else if (curr_seatbelt == (int) 0){
    LCD_slave.write(212);
  }
}

int check_seatbelt(int init_seatbelt){
  int curr_seatbelt;
  if ((digitalRead(opto_pin) == LOW) && (duty_cycle > 25)){
    curr_seatbelt = 1;
  } else  {
    curr_seatbelt = (int) 0;
  }
  if (curr_seatbelt != init_seatbelt){
    write_seatbelt(curr_seatbelt);
    return curr_seatbelt;
  } else {
    return init_seatbelt;
  }
}

void loop() {
  // reset everything
    forward_flag = check_dir(forward_flag);
    delay(10);
    duty_cycle = check_duty_cycle(duty_cycle);
    delay(10);
    seatbelt_flag = check_seatbelt(seatbelt_flag);
    delay(10);
    steer_angle = check_steering(steer_angle);  
  // push button pressed, and toggle switch is on neutral
  if ((digitalRead(push_pin) == LOW) && (forward_flag == 'n') && start_flag == 0){
    Serial.println("Push button pressed!");
    ref_time = millis();
    curr_time = millis();
    while ((curr_time - ref_time) <= 1000){
      curr_time = millis();
      if (digitalRead(push_pin) == HIGH){
        // push button is released, reset program!
        Serial.println("Program reset!");
        start_flag = 0;
        return;
      }
    }
    ref_time = 0;
    tone(buzzer_pin, 500, 100);
    start_flag = 1;
    Serial.println("Start car!");
    delay(300);
  } else if ((start_flag == 1) && (digitalRead(push_pin) == LOW)){
    Serial.println("Stop!");
    digitalWrite(red_led, HIGH);
    start_flag = 0;
    direction(forward_flag, 0);
    noTone(buzzer_pin);
    return;
  }

  // exit while loop if push button is pressed
  if (start_flag == 1){
    forward_flag = check_dir(forward_flag);
    delay(10);
    duty_cycle = check_duty_cycle(duty_cycle);
    delay(10);
    seatbelt_flag = check_seatbelt(seatbelt_flag);
    delay(10);
    steer_angle = check_steering(steer_angle);    
    if (forward_flag == 'r'){
      sound_buzzer();
    } else {
      noTone(buzzer_pin);
    }
    if ((digitalRead(push_pin) == HIGH) && (start_flag == 1)){
      direction(forward_flag, throttle);
    
      if (forward_flag != 'n'){
        if (seatbelt_flag){
          // seatbelt not worn and throttle is above 25%
          blink_led();
      } else {
        digitalWrite(red_led, HIGH);
    }
  } else {
    digitalWrite(red_led, HIGH);
  }
}    
  }

}
