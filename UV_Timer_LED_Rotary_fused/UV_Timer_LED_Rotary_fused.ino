/**
   ULTRAVIOLET EXPOSURE UNIT FIRMWARE - E DAVIDSON MAY 2018
   Uses relay to switch on 12v led strips
   input rotary encoder
   input thermistor for safety

   three pins out to 74HC595 for common-anode seven-segment display
   three pins out to control individual digits and three pins

*/

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxx - PAUSE FUNCTIONS START

bool hot = false;
bool warm = false;

int go_Button = A4;

const int LEDOrange = A5;
const int LEDRed = A2;
const int LEDGreen = A3;
const int LEDBlue = 10;

const int relay = 9;
const int fan = 4;
int reed = A0;
int therm = A1;

//This leaves A6 and A7 which are compulsory Analog inputs

unsigned int counter = 0;
int temperature = 0;

//xxxxxxxxxxxxxxxx - PAUSE FUNCTIONS END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxx - 7 SEGMENT DISPLAY CODE - BEGIN

const int bit_clock_pin = 7;  // pin 11 of 74HC595 (SHCP)
const int digit_clock_pin = 6;  // pin 12 of 74HC595 (STCP)
const int data_pin = 5; // pin 14 of 74HC595 (DS)

const int LED_ONES = 13;  //positions of digits - these got mixed up
const int LED_TENS = 11;
const int LED_HUND = 12;

const byte digit_pattern[11] =  // digit pattern - 7-seg disp
{
  B01111110,  // 0
  B01001000,  // 1
  B10111010,  // 2
  B11101010,  // 3
  B11001100,  // 4
  B11100110,  // 5
  B11110110,  // 6
  B01001010,  // 7
  B11111110,  // 8
  B11001110,  // 9
  B10010110,  // F
};

//xxxxxxxxxxxxxxxx - 7 SEGMENT DISPLAY CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxx - ROTARY ENCODER CODE - BEGIN


const int  encoderPinA = 3;   // right (labeled DT on our decoder, yellow wire)
const int encoderPinB = 2;   // left (labeled CLK on our decoder, green wire)
const int clearButton = 8;    // switch (labeled SW on our decoder, orange wire)

volatile unsigned int encoderPos = 0;  // a counter for the dial
unsigned int lastReportedPos = 1;   // change management
static boolean rotating = false;    // debounce management


boolean A_set = false;  // interrupt service routine vars
boolean B_set = false;

//xxxxxxxxxxxxxxxx - ROTARY ENCODER CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
void setup()  //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //xxxxxxxxxxxxxxxx - 7 SEGMENT DISPLAY CODE - BEGIN

  pinMode(data_pin, OUTPUT);
  pinMode(bit_clock_pin, OUTPUT);
  pinMode(digit_clock_pin, OUTPUT);
  pinMode(LED_ONES, OUTPUT); //ones
  pinMode(LED_TENS, OUTPUT); //tens
  pinMode(LED_HUND, OUTPUT); //hundreds

  //xxxxxxxxxxxxxxxx - 7 SEGMENT DISPLAY CODE - END
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //xxxxxxxxxxxxxxxx - ROTARY ENCODER CODE - BEGIN

  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);
  digitalWrite(clearButton, HIGH);

  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);
  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);

  //xxxxxxxxxxxxxxxx - ROTARY ENCODER CODE - END
  //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //xxxxxxxxxxxxxxxx - PAUSE FUNCTIONS START
  pinMode(go_Button, INPUT_PULLUP);
  
  pinMode(relay, OUTPUT); //relay 
  
  pinMode(therm, INPUT); //thermistor
    
  pinMode(reed, INPUT); //thermistor
  digitalWrite(reed, HIGH);
  
  pinMode(fan, OUTPUT);
  
  pinMode(LEDOrange, OUTPUT);
  pinMode(LEDRed, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  pinMode(LEDBlue, OUTPUT);

 


  //xxxxxxxxxxxxxxxx - PAUSE FUNCTIONS END

  //  Serial.begin(9600); // initialize serial comms @ 9600-BPS
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop()  //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

//  digitalWrite(LEDOrange, HIGH);
//  delay (5);
//  digitalWrite(LEDOrange, LOW);
//  // //then
//  digitalWrite(LEDRed, HIGH);
//  delay (5);
//  digitalWrite(LEDRed, LOW);
//  // //then
//  digitalWrite(LEDGreen, HIGH);
//  delay (5);
//  digitalWrite(LEDGreen, LOW);
//  // //then
//  digitalWrite(LEDBlue, HIGH);
//  delay (5);
//  digitalWrite(LEDBlue, LOW);
//  //then
//  digitalWrite(fan, HIGH);
//  delay (3000);
//  digitalWrite(fan, LOW);
//  delay (3000);
//


  
  rotating = true;  // reset the debouncer

  if (lastReportedPos != encoderPos) {
    //   Serial.print("Index:");
    //   Serial.println(encoderPos, DEC);
    lastReportedPos = encoderPos;
  }
  if (digitalRead(clearButton) == LOW )  {
    encoderPos = 0;
  }

  if (encoderPos >= 599 && encoderPos <= 800)
  {
    encoderPos = 999;
  }
  else if (encoderPos >= 801)
  {
    encoderPos = 0;
  }

 
// Always check temperature
//checkTemp();

  disp_3_digit(encoderPos);

//then trigger go button
  
if (digitalRead(reed) == LOW)
  {
    digitalWrite(LEDBlue, LOW);
    if (digitalRead(go_Button) == LOW)
    {
      go_Pause();
    }
  }
else if (digitalRead(reed) == HIGH)
  {
    digitalWrite(LEDBlue, HIGH);
  }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++ - FUNCTIONS - ++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

//xxxxxxxxxxxxxxxx - 7 SEGMENT DISPLAY CODE - BEGIN

void update_one_digit(int data)
// DO ONE DIGIT - THIS METHOD IS CALLED BY THE
{
  int i;
  byte pattern;
  pattern = digit_pattern[data];// get binary for number
  digitalWrite(digit_clock_pin, LOW);  // turn off 74HC595 output
  shiftOut(data_pin, bit_clock_pin, MSBFIRST, ~pattern); // update 74HC595
  digitalWrite(digit_clock_pin, HIGH); // turn on 74HC595
}
// THREE DIGIT DISPLAY METHOD BELOW
void disp_3_digit(int numb) // DO THREE DIGITS
// THIS WILL DO THE FULL DISPLAY WRITE BY BREAKING DOWN THE NUMBER
// AND THEN CALLING THE SINGLE DIGIT METHOOD FOR EACH DIGIT
{
  int ones = numb % 10;     //break number out to individual digits
  int tens = (numb % 100) / 10;
  int hund = numb / 100;
  //
  digitalWrite(LED_HUND, HIGH); //output each digit
  update_one_digit(ones);
  digitalWrite(LED_HUND, LOW);
  //
  digitalWrite(LED_TENS, HIGH);
  update_one_digit(tens);
  digitalWrite(LED_TENS, LOW);
  //
  digitalWrite(LED_ONES, HIGH);
  update_one_digit(hund);
  digitalWrite(LED_ONES, LOW);
}

//xxxxxxxxxxxxxxxx - 7 SEGMENT DISPLAY CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxx - ROTARY ENCODER CODE - begin

// Interrupt on A changing state
void doEncoderA() {
  // debounce
  if ( rotating ) delay (5);  // wait a little until the bouncing is done

  // Test transition, did things change?
  if ( digitalRead(encoderPinA) != A_set ) { // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if ( A_set && !B_set )
      encoderPos ++;

    rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB() {
  if ( rotating ) delay (5);
  if ( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( B_set && !A_set )
      encoderPos --;

    rotating = false;
  }
}
//xxxxxxxxxxxxxxxx -  ROTARY ENCODER CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxx - PAUSE FUNCTIONS START

  void go_Pause()   // - [GO/PAUSE BUTTON] - GO FUNCTION
{

if (encoderPos>0)
{
  delay(1000);
  counter = (encoderPos);
  digitalWrite(LEDGreen, HIGH);
  digitalWrite(relay, HIGH);
  
  
    while (counter > 0) //Seconds Display Loop
      {
        int i = 97;
        checkTemp();
        while (i > 0) // 99 loops each second
          {  
            disp_3_digit(counter);  //display
            delay(10);              //wait 10ms
            // Now check stuff - every 12ms is fine
            if (digitalRead(go_Button) == LOW)
              {
                Pause(); // CALL FUNCTION BELOW IF 
               }         // GO/PAUSE BUTTON PRESSED
            else;
            i -=1;
          }
        counter -=1;
      }
   digitalWrite(relay, LOW);
   digitalWrite(LEDGreen, LOW);
  }
}
// FUNCTION FOR PAUSING THE EXPOSURE

void Pause()   // - [GO/PAUSE BUTTON] - PAUSE FUNCTION
{
  int keep = 0; //keep track of light flashing
  
  digitalWrite(LEDOrange, HIGH);
  digitalWrite(LEDRed, HIGH);
  digitalWrite(LEDGreen, HIGH);
  digitalWrite(LEDBlue, HIGH);
  
  digitalWrite(relay, LOW);   //turn off UV light
  delay(1000); //force minimum one second pause for debounce

//  structural stuff - turn stuff off -- pause - then...

// MAKE LOOP WHAT HAPPENS OF BUTTON NOT PRESSED

  if (digitalRead(go_Button) != LOW) //check pause button
  {
// FIRST INDICATOR LEDS

// TIME KILLING LOOP TO MAKE THE LIGHT FLASH
    for (int i = 0;i=25;i++)
      {
        checkTemp();
        if (digitalRead(go_Button) == LOW)
          {
            //IF BROKEN OUT OF LOOP RETURN TO GO LOOP
            delay(1000); //force minimum one second pause for debounce
            digitalWrite(relay, HIGH);
            //digitalWrite(LEDGreen, HIGH);
              digitalWrite(LEDOrange, LOW);
              digitalWrite(LEDRed, LOW);
              digitalWrite(LEDBlue, LOW);
             break; //i=25; // Just break out if button pressed
          }
        else
          {
            checkTemp();    
            delay(20);  //wait a bit
          }
       }
//LOOP BACK TO CHECK STATE OF PAUSE BUTTON
   }
}


//
void checkTemp()
{
  int sensorValue = analogRead(A0);
}




