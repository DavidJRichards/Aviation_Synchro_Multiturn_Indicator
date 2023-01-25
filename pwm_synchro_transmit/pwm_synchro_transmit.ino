// 3 phase PWM sine
// (c) 2016 C. Masenas
// Modified from original DDS from: 
// KHM 2009 /  Martin Nawrath
// djrm, modified into synchro transmitter
// wip: 22 Jan 2023

#include <TM1637.h>
// define a module on data pin A2, clock pin A1
TM1637 module(A2, A1);  

// table of 256 sine values / one sine period / stored in flash memory
PROGMEM const unsigned char sine256[]  = {
  
  // 0 to 90    (0-63)
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,
  217,219,221,223,225,227,229,231,233,234,236,238,239,240,242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,
  // 90 to 180  (64-127)
  254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,221,219,
  217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,

  // 180 to 270 (128-191)
  127,124,121,118,115,111,108,105,102, 99, 96, 93, 90, 87, 84, 81, 78, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
   37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 16, 15, 14, 12, 11, 10,  9,  7,  6,  5,  5,  4,  3,  2,  2,  1,  1,  1,  0,  0,  0,
  // 270 to 360 (192-255)
    0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  5,  6,  7,  9, 10, 11, 12, 14, 15, 16, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
   37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76, 78, 81, 84, 87, 90, 93, 96, 99,102,105,108,111,115,118,121,124

};
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

int TOGGLE = 1;
int testPin = 7;
//int enablePin = 6 ;
int syncIp = 3; // interrupt input to synchronise frequency generation
int Freq_IN = A0;
int potVal = 200;
int newVal;
byte phaseVal;

volatile  float freq=1;
const float refclk=122.549  ;     //  16 MHz/510/256

// variables used inside interrupt service declared as voilatile
volatile unsigned long sigma;   // phase accumulator
volatile unsigned long delta;  // phase increment

// phase 0,1,2 are instatntanious 400Hz wave angles used to make AC waveform 
byte phase0, phase1, phase2;
//byte phase0inv, phase1inv;

// phaseA,B,c Are rotor angle used to modify rotor voltage values 
byte phaseA, phaseB, phaseC;

void setup()
{
  module.setupDisplay(true, 3);
  module.setDisplayToString("DJR");
  delay(400);
  module.clearDisplay();

  Serial.begin(9600);        // connect to the serial port
  Serial.println("DDS Test");

//  pinMode(enablePin, OUTPUT);      // sets the digital pin as output
  pinMode(testPin, OUTPUT);      // sets the digital pin as output
  pinMode(Freq_IN, INPUT);
  pinMode(9, OUTPUT);     // pin9= PWM  output / frequency output
  pinMode(10, OUTPUT);     // pin10= PWM  output / frequency output
  pinMode(11, OUTPUT);     // pin11= PWM  output / frequency output

  Setup_timer2();
  Setup_timer1();
//  digitalWrite(enablePin, HIGH);
  attachInterrupt(digitalPinToInterrupt(syncIp), zerocrossing, FALLING); 

// the waveform index is the highest 8 bits of sigma
// choose refclk as freq to increment the lsb of the 8 highest bits
//    for every call to the ISR of timer2 overflow
// the lsb of the 8 highest bits is 1<<24 (1LL<<24 for long integer literal)
  delta = (1LL<<24)*freq/refclk ;  
  changeFreq(400);      // operate at fixed frequency of 400Hz
}

void loop(){
  char text[4];

// update rotor angle from pot connected to analogue 0 input  
  if( (potVal=analogRead(Freq_IN)) != newVal)
  {
    newVal=potVal;
    if(potVal>1020)potVal=1020;
    phaseVal=map(potVal,0,1020,0,255);

  sprintf(text, "%3u", map(potVal,0,1020,0,360));
  module.setDisplayToString(text);
  delay(100);

    if (TOGGLE)
    {
      Serial.print(potVal);
      Serial.print(" ");
      Serial.print(phaseVal);
      Serial.print(" ");

      Serial.print(phaseA);
      Serial.print(" ");
      Serial.print(phaseB);
      Serial.print(" ");
      Serial.println(phaseC);
    }
  }
              
}

void zerocrossing()          
{                   
   sigma = 32L << 24;              // sync generator to zero crossing
}

void changeFreq(float _freq){
  cbi (TIMSK2,TOIE2);              // disable timer2 overflow detect
  freq = _freq;
  delta=(1LL<<24)*freq/refclk;  // update phase increment
  sbi (TIMSK2,TOIE2);              // enable timer2 overflow detect
} 

//******************************************************************
// timer2 setup
// set prscaler to 1,  fast PWM
void Setup_timer2() {

// Timer2 Clock Prescaler to : 1
  sbi (TCCR2B, CS20);  // set
  cbi (TCCR2B, CS21);  // clear
  cbi (TCCR2B, CS22);

  // Timer2 PWM Mode 
  cbi (TCCR2A, COM2A0);  // clear OC2A on Compare Match, PWM pin 11
  sbi (TCCR2A, COM2A1);

  // set to fast PWM
  sbi (TCCR2A, WGM20);  // Mode 1, phase correct PWM
  cbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);

  sbi (TIMSK2,TOIE2);              // enable overflow detect
  
}
// timer1 setup  (sets pins 9 and 10)
// set prscaler to 1, PWM mode to phase correct PWM,  16000000/510 = 31372.55 Hz clock
void Setup_timer1() {

// Timer1 Clock Prescaler to : 1
  sbi (TCCR1B, CS10);
  cbi (TCCR1B, CS11);
  cbi (TCCR1B, CS12);

  // Timer1 PWM Mode set to Phase Correct PWM
  cbi (TCCR1A, COM1A0);  // clear OC1A on Compare Match, PWM pin 9
  sbi (TCCR1A, COM1A1);
  cbi (TCCR1A, COM1B0);  // clear OC1B on Compare Match, PWM pin 10
  sbi (TCCR1A, COM1B1);

  sbi (TCCR1A, WGM10);  // Mode 1  / phase correct PWM
  cbi (TCCR1A, WGM11);
  cbi (TCCR1B, WGM12);
  cbi (TCCR1B, WGM13);
}

//******************************************************************
// Timer2 Interrupt Service at 31372,550 KHz = 32uSec
// this is the timebase REFCLOCK for the DDS generator
// runtime : 8 microseconds ( inclusive push and pop)
// OC2A - pin 11
// OC1B - pin 10
// OC1A - pin 9
// https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM
ISR(TIMER2_OVF_vect) {

  byte idx;                       // read value fron ROM sine table and send to PWM DAC
  byte amp;

  sigma = sigma + delta;          // soft DDS, phase accu with 32 bits
  phase0 = sigma >> 24;           // use upper 8 bits for phase accu as frequency information

  phase1 = phase0;          
  phase2 = phase0;        

  // generate 400 Hz reference +ve going pulse for syncing rotor power supply 
  if(phase2 < 10)       // tradeoff between good sync and waveform distortion here
  cbi(PORTD,testPin);   
  else
  sbi(PORTD,testPin);          
  
//-----------------------------------------------------------------------------------------
// generate two signals 120 degrees apart
// to feed into S1 and S2, S3 being common ground.
 #if 1
  idx = 128 | phaseVal;                             // rotation angle (0 o 255)
  phaseA = 2 * pgm_read_byte_near(sine256 + idx);   // sin rotation angle is scale factor

  if(phaseVal < 64 || phaseVal > 192)               // invert when needed
    phase0 = phase0 + 128;                          // invert phase of drive signal

  amp = pgm_read_byte_near(sine256 + phase0);       // get unscaled instantanious output voltage
  idx = amp * phaseA / 256;                         // scale output value by amplitude factor
  OCR2A = idx;                                      // pwm pin 11

//------------------------------------------------------------------------------------------
 
  idx = 128 | phaseVal + 85;                        // shifted rotation angle (120 deg)
  phaseB = 2 * pgm_read_byte_near(sine256 + idx);

  if(phaseVal < 112 || phaseVal > 232)              
    phase1 = phase1 + 128;                          // invert phase of drive signal

  amp = pgm_read_byte_near(sine256 + phase1);
  idx = amp *  phaseB / 256;
  OCR1B = idx;                                      // pwm pin 10

//------------------------------------------------------------------------------------------
 
  idx = 128 | phaseVal - 85;                        // shifted rotation angle (240 deg)
  phaseC = 2 * pgm_read_byte_near(sine256 + idx);  

  if(phaseVal < 28 || phaseVal > 148)              
    phase2 = phase2 + 128;                          // invert phase of drive signal

  amp = pgm_read_byte_near(sine256 + phase2);
  idx = amp *  phaseC / 256;
  OCR1A = idx;                                      // pwm pin 9

#else
//------------------------------------------------------------------------------------------
// alternative is to generate full amplitude three phase signal 
  OCR2A=pgm_read_byte_near(sine256 + phase0);       // pwm pin 11
  OCR1B=pgm_read_byte_near(sine256 + phase1);       // pwm pin 10
  OCR1A=pgm_read_byte_near(sine256 + phase2);       // pwm pin 9
#endif  

}