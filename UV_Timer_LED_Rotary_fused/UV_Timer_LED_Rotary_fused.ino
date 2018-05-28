/** 
   ULTRAVIOLET EXPOSURE UNIT FIRMWARE - E DAVIDSON MAY 2018
   Uses relay to switch on 12v led strips
   input rotary encoder
   input thermistor for safety
   
   three pins out to 74HC595 for common-anode seven-segment display
   three pins out to control individual digits and three pins
   
*/


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                7 SEGMENT DISPLAY CODE - BEGIN
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

const int bit_clock_pin = 7;  // pin 11 of 74HC595 (SHCP)
const int digit_clock_pin = 6;  // pin 12 of 74HC595 (STCP)
const int data_pin = 5; // pin 14 of 74HC595 (DS)

const int LED_ONES = 11;  //PINS
const int LED_TENS = 13;
const int LED_HUND = 12;

const byte digit_pattern[10] =  // digit pattern - 7-seg disp
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
};
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                7 SEGMENT DISPLAY CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                ROTARY ENCODER CODE - BEGIN
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

const int  encoderPinA = 2;   // right (labeled DT on our decoder, yellow wire)
const int encoderPinB = 3;   // left (labeled CLK on our decoder, green wire)
const int clearButton = 8;    // switch (labeled SW on our decoder, orange wire)

volatile unsigned int encoderPos = 0;  // a counter for the dial
unsigned int lastReportedPos = 1;   // change management
static boolean rotating = false;    // debounce management


boolean A_set = false;  // interrupt service routine vars
boolean B_set = false;
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                ROTARY ENCODER CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

unsigned int counter = 0;
int button = A4;
int therm = A1;
int thermout = A2;

void setup()  //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
{
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                7 SEGMENT DISPLAY CODE - BEGIN
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  pinMode(data_pin, OUTPUT);
  pinMode(bit_clock_pin, OUTPUT);
  pinMode(digit_clock_pin, OUTPUT); 
  pinMode(LED_ONES, OUTPUT); //ones
  pinMode(LED_TENS, OUTPUT); //tens
  pinMode(LED_HUND, OUTPUT); //hundreds
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                7 SEGMENT DISPLAY CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                ROTARY ENCODER CODE - BEGIN
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);
  digitalWrite(clearButton, HIGH);

    // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);
  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                ROTARY ENCODER CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  pinMode(button, INPUT_PULLUP); 
  pinMode(A3, OUTPUT); //relay
  pinMode(A1, OUTPUT); //thermistor
  pinMode(A2, INPUT); //thermistor
//  Serial.begin(9600); // initialize serial comms @ 9600-BPS
}




void loop()  //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
{ 
  rotating = true;  // reset the debouncer

  if (lastReportedPos != encoderPos) {
 //   Serial.print("Index:");
 //   Serial.println(encoderPos, DEC);
    lastReportedPos = encoderPos;
  }
  if (digitalRead(clearButton) == LOW )  {
    encoderPos = 0;
}

if (encoderPos>=599&&encoderPos<=800) 
{
  encoderPos = 999;
} 
else if (encoderPos>=801) 
{
  encoderPos = 0;
}
disp_3_digit(encoderPos);

      if (digitalRead(button) == LOW)
      {
        go_Pause();
      }
}




//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                7 SEGMENT DISPLAY CODE - BEGIN
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//XXX - UPDATE INFO OF THE LED DISPLAY - XXX

void update_one_digit(int data) 
    // DO ONE DIGIT - THIS METHOD IS CALLED BY THE 
    // THREE DIGIT DISPLAY METHOD BELOW                           
{
  int i;
  byte pattern;
  pattern = digit_pattern[data];// get binary for number
  digitalWrite(digit_clock_pin, LOW);  // turn off 74HC595 output
  shiftOut(data_pin, bit_clock_pin, MSBFIRST, ~pattern); // update 74HC595
  digitalWrite(digit_clock_pin, HIGH); // turn on 74HC595
}

void disp_3_digit(int numb) // DO THREE DIGITS
    // THIS WILL DO THE FULL DISPLAY WRITE BY BREAKING DOWN THE NUMBER
    // AND THEN CALLING THE SINGLE DIGIT METHOOD FOR EACH DIGIT
{
int ones = numb%10;       //break number out to individual digits
int tens = (numb%100)/10;
int hund = numb/100;
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
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                7 SEGMENT DISPLAY CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                ROTARY ENCODER CODE - begin
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
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
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                ROTARY ENCODER CODE - END
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

  void go_Pause()
{
digitalWrite(A3, HIGH);
counter = encoderPos;

  while (counter > 0) //seconds
    {
      int i = 97;
      while (i > 0) // 99 loops each second
        {  
          disp_3_digit(counter);  //display
          delay(10);              //wait 10ms
          i -=1;
        }
      counter -=1;
    }
 digitalWrite(A3, LOW);
}

