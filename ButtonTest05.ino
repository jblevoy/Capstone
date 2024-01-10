
//test having two buttons on an interrupt, 

//SCL = CLK for Displays
//SDA = Display 1
//13 = Display 2
//12 = Display 3
//11 = LED Blue - PWM
//10 = LED Blue - PWM
//9 = LED Blue - PWM
//8 = Cooling - PWM
//7 = TC DAT
//6 = Mixing - PWM
//5 = Button 1 - PWM
//4 = Button 2
//3 = Button 3 - PWM, Interrupt
//2 = Button Check ISR - Interrupt 

long int timer1 = 0;
long int timer2 = 0;
long int timer3 = 0;

long int counter1 = 0; //count up button1
long int counter2 = 0; //count down button2
long int counter3 = 0; //timer reset

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP); //B1 temp up. 
  pinMode(3, INPUT_PULLUP); //
  pinMode(4, INPUT_PULLUP); //
  pinMode(5, INPUT_PULLUP); //
  attachInterrupt(digitalPinToInterrupt(2), debounce, LOW);
}

void loop() {
  //Serial.println(counter);
  //delay(2000);
  Serial.print("counter1: ");
  Serial.println(counter1);
  //Serial.print("counter3: ");
  //Serial.println(counter3);
  delay(1000);
  //digitalRead(4);
}

void debounce() {
  Serial.println("in debounce");
  Serial.println(timer1);
  Serial.println(millis());
  Serial.println(millis() - timer1);
  if (digitalRead(5) == 0 && millis() - timer1 >= 60){
    Serial.println("in counter1++");
    counter1++;
    timer1 = millis();
  }
  /*
  if (digitalRead(4) == 0 && millis() - timer2 >= 60){
    counter1--;
    timer2 = millis();
  }
  if (digitalRead(3) == 0 && millis() - timer3 >= 60 ){
    counter3++;
    timer3 = millis();
  }
  */
}
