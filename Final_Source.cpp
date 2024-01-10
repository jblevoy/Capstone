//Combined TCs, PWM, displays, RGB LED, Stepper for temp reg test
//Includes the Arduino Stepper Library

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

//A0 = IN4 stepper
//A1 = IN3 stepper
//A2 = IN2 stepper
//A3 = IN1 stepper
//A5 = free

#include <Stepper.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <Arduino.h> //displays
#include <TM1637TinyDisplay.h>

#include <TimeLib.h> //for timer library based on millis

#define CLK_1 SCL //displays
#define DIO_1 SDA
#define DIO_2 13
#define DIO_3 12

// Data wire is connected to the Arduino digital pin 4 for thermocouple
#define ONE_WIRE_BUS 7

// Setup a oneWire instance to communicate with any OneWire devices (thermocouple)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor (thermocouple)
DallasTemperature sensors(&oneWire);

float plate = 0; //Initiate var for plate TC to 0
int plateSet = 3; //Initiate var for pre-set plate temp for TC, stepper regulation
int tempSet = 6; //Initiate target temp var for container
int plateHysterisis = 1; //How much it overshoots. zero means only thermal inertia of HE plate is creating delay between actuator cycles
int contHysterisis = 0.5;
float containerLeft = 0; //Initiate var for container TC left to 0
float containerRight = 0; //Initiate var for container TC right to 0
int coolantVoltage = 100; //Initiate cooling PWM duty cycle to 100/255
float contAve = 0; //Initiate container thermocoulple average
float maxDiff = 1; //difference between thermocouples before mixing comes on

// Defines the number of steps per rotation
const int stepsPerRevolution = 2038; //includes 64:1 gear ratio

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, A3, A1, A2, A0); //library function Stepper initializes analog pins A3-A0 
int stepSpeed = 16;

bool down = false; //is tray down already?

// Instantiate TM1637TinyDisplay Class for display
TM1637TinyDisplay display1(CLK_1, DIO_1);  //display1 and display 2 are objects of the TM1637TinyDisplay class
TM1637TinyDisplay display2(CLK_1, DIO_2);  //display1 and display 2 are objects of the TM1637TinyDisplay class
TM1637TinyDisplay display3(CLK_1, DIO_3);  //display1 and display 2 are objects of the TM1637TinyDisplay class

long int timer1 = 0; //count up button timer, start tempSet at 6 C
long int timer2 = 0; //count down button timer
long int timer3 = 0; //reset button timer
long int timer4 = 0; //timer for plate anti-ambivalence
long int timer5 = 0; //timer for led anti-ambivalence
long int timer6 = 0; //timer for cooling anti-ambivalence
long int timer7 = 0; //timer for mixing anti-ambivalence
long int transitDuration = 0; //count up timer for front panel
long int resetTime1 = 0; //initialize resetTime to beginning of program. will get updated to millis if button is pressed
long int seconds60 = 0; //for transitDuration
long int minutes = 0;

void setup() {
  Serial.begin(9600);
  //Start up the OneWire library
  sensors.begin();
  
  myStepper.setSpeed(stepSpeed); //home to ceiling
	myStepper.step(-3000); //CW to lift tray, number of steps given as arg
  myStepper.setSpeed(stepSpeed); //CCW to lower tray, home seq pt 2: lower slightly
	myStepper.step(200); //CCWto lower tray, number of steps given as arg
  
  pinMode(11, OUTPUT); //RGB Blue
  pinMode(10, OUTPUT); //RGB Green
  pinMode(9, OUTPUT); //RGB Red

  // Initialize Display objects
  display1.begin();
  display2.begin();
  display3.begin();

  pinMode(4, INPUT_PULLUP); //B1 temp up
  pinMode(3, INPUT_PULLUP); //B2 temp down, has interrupt capability if that end up helping
  pinMode(2, INPUT_PULLUP); //B3 reset, has interrupt capability if that end up 
  
  pinMode(5, INPUT_PULLUP); //top button (button1)
  attachInterrupt(digitalPinToInterrupt(2), debounce, LOW);
}

void loop() {
  //Serial.print("tempSet: ");
  //Serial.println(tempSet);
  //delay(2000);

  sensors.requestTemperatures(); //this line by itself seems to add about a second to loop time -_-
  plate = sensors.getTempCByIndex(2);  //Chamber A - can has more than one IC on the same bus. 0 refers to the first IC on the wire
  //Serial.print("plate: ");
  //Serial.println(plate);
  containerLeft = sensors.getTempCByIndex(1);  //Chamber A - can has more than one IC on the same bus. 0 refers to the first IC on the wire
  //Serial.print("containerLeft: ");
  //Serial.println(containerLeft);
  containerRight = sensors.getTempCByIndex(0);  //Chamber A - can has more than one IC on the same bus. 0 refers to the first IC on the wire
  //Serial.print("containerRight: ");
  //Serial.println(containerRight);

  //Serial.print("plateSet: ");
  //Serial.println(plateSet);
  
  if (plate > (plateSet + plateHysterisis) && down == false && millis() - timer4 > 5000 ){ //lower tray	
    //Serial.println("are we in red?");
    myStepper.setSpeed(stepSpeed); //set speed
	  myStepper.step(2000); //CCW to lower tray, number of steps given as arg
    down = true;
    timer4 = millis();
  }
  
  if (plate <= (plateSet - plateHysterisis) && down == true && millis() - timer4 > 5000 ){ //lift tray	
    myStepper.setSpeed(stepSpeed); //set speed
	  myStepper.step(-2000); //CW to lift tray, number of steps given as arg
    down = false;
    timer4 = millis();
  }

  //Serial.println()
  contAve = (containerLeft+containerRight)/2; //take average of container for use in regulation tasks
  //Serial.print("contAve: ");
  //Serial.println(contAve);

  if (contAve >= (tempSet+1) && (millis() - timer5) > 5000){ //turn on red if overtemp
    //Serial.println("are we in over?");
    analogWrite(11, 255); //RGB Blue, zero means full on
    analogWrite(10, 255); //RGB Green
    analogWrite(9, 150); //RGB Red
    timer5 = millis(); //anti-ambivalence measure, 5 sec
  }
  if (abs(contAve - tempSet) < 1 && (millis() - timer5) > 5000){ //turn just green on if nominal
    analogWrite(11, 255); //RGB Blue, zero means full on
    analogWrite(10, 150); //RGB Green
    analogWrite(9, 255); //RGB Red
    timer5 = millis();
  }
  if (contAve <= (tempSet-1) && (millis() - timer5) > 5000){ //turn just blue on undertemp
    analogWrite(11, 150); //RGB Blue, zero means full on
    analogWrite(10, 255); //RGB Green
    analogWrite(9, 255); //RGB Red
    timer5 = millis(); 
  }

  organTimer(); //call organ timer function to update timer

  display1.showNumber(contAve); //show container TC average 
  display2.showNumber(tempSet); //show tempSet pre-set temp for container
  
  if (contAve > (tempSet + contHysterisis) && millis() - timer6 > 5000){ //take average of container TCs for coolant pump on
    //Serial.println("are we in turn coolant on");
    analogWrite(8, 150);
    timer6 = millis();
  }
  if (contAve <= (tempSet - contHysterisis) && millis() - timer6 > 5000){ //coolant pump off
    analogWrite(8, 0); //turn off coolant
    timer6 = millis();
  }
  
  if (abs(containerLeft-containerRight) > maxDiff && millis() - timer7 > 5000){ //mixing pump on
    //Serial.println("are we in turn mixing on");
    analogWrite(6, 200);
    timer7 = millis();
  }
  if (abs(containerLeft-containerRight) <= maxDiff && millis() - timer7 > 5000){ //mixing pump off
    analogWrite(6, 0); //turn off mixing
    timer7 = millis();
  }
  
  //Serial.print("abs(containerLeft-containerRight: ");
  //Serial.println(abs(containerLeft-containerRight));
} //end main loop

void debounce() {
  //Serial.println("in debounce");
  //Serial.println(timer1);
  //Serial.println(millis());
  //Serial.println(millis() - timer1);
  if (digitalRead(5) == 0 && millis() - timer1 >= 60){ //count up tempSet
    //Serial.println("in tempSet++");
    tempSet++;
    display2.showNumber(tempSet); //show tempSet
    timer1 = millis();
  }
  if (digitalRead(4) == 0 && millis() - timer2 >= 60){ //count down tempSet
    //Serial.println("in tempSet--");
    tempSet--;
    display2.showNumber(tempSet); //show tempSet
    timer2 = millis();
  }
  if (digitalRead(3) == 0 && millis() - timer3 >= 60){ //
    //Serial.println("in reset");
    timer3 = millis();
    if(resetTime1 == 0){
      resetTime1 = transitDuration;
    }
    if(resetTime1 != 0){
      transitDuration = transitDuration - resetTime1;
      display3.showNumberDec(transitDuration, 0b11100000, true, 4, 0); //showNumberDecEx(number,dots,leading_zeros,length,begin char position      
    }
  }
}

void organTimer(){
  time_t t = now(); // Store the current time in var t
  seconds60 = t % 60;
  minutes = minute(t);  
  transitDuration = minutes*100+seconds60;
  if(resetTime1 != 0){
    transitDuration = transitDuration - resetTime1;
  }
  if (transitDuration%5==0){ //make sure it only updates display every 5 sec, because doing it every second is visibly choppy
    display3.showNumberDec(transitDuration, 0b11100000, true, 4, 0); //showNumberDecEx(number,dots,leading_zeros,length,begin char position
  }
  //Serial.print("transitDuration: ");
  //Serial.println(transitDuration);
}


