// provides dimmable led array or other DC pulse-modulated by linear 10k pot. When slider/pot has been
// inactive for a while, turn off automatically.  Then use PIR sensor to switch on & off until
// slider is used again, which resets the normal use mode timer. 
// Uses I2C motor driver from Seeed Studio with both outputs tied together.
// Using at higher power than Seeed's spec but not according to the L298 data sheet.  
// Kind of a waste for this non-inductive load, but
// the plan is to add more functions and input sensors to the mini-pro, so the I2C saves time.
// Both L298 and voltage regulator are heatsinked to aluminium square tubes, top & bottom.
// surpluss power brick is limited at about 2.5 amps, 19.5 volts.

#include <Wire.h>

#define I2CMotorDriverAdd         0x0f   // Set the address of the I2CMotorDriver

#define MotorSpeedSet             0x82
#define PWMFrequenceSet           0x84
#define DirectionSet              0xaa
#define MotorSetA                 0xa1
#define MotorSetB                 0xa5
#define Nothing                   0x01

byte motionPin = 9;    // for PIR sensor output pin connection
boolean alert = LOW;        // track High/Low for motion
boolean lastAlert = LOW;
int sensorPin = A0;    //  input pin for the potentiometer / slider
int ledPin = 13;      //  pin for the LED
int sensorValue = 0;  // variable to store the value coming from the slider
int oldSensorVal = 0; // for comparison
int dull = 10;        // delta required to cause action
int blinkrate = 0;   // sets the led and main lights 'on' time in each duty cycle
//unsigned long goodnight = 7200000;  // wait for these miliseconds of inactivity, then turnoff
unsigned long goodnight = 7200000;    // entering PIR sensor mode.
unsigned long countdown = goodnight; // 60*60*2hr*1000ms

void MotorSpeedSetAB(unsigned char MotorSpeedA , unsigned char MotorSpeedB)  {
  MotorSpeedA=map(MotorSpeedA,0,100,0,255);
  MotorSpeedB=map(MotorSpeedB,0,100,0,255);
  Wire.beginTransmission(I2CMotorDriverAdd); // transmit to device I2CMotorDriverAdd
  Wire.write(MotorSpeedSet);        // set pwm header 
  Wire.write(MotorSpeedA);              // send pwma 
  Wire.write(MotorSpeedB);              // send pwmb    
  Wire.endTransmission();    // stop transmitting
}

void MotorPWMFrequenceSet(unsigned char Frequence)  {    
  Wire.beginTransmission(I2CMotorDriverAdd); // transmit to device I2CMotorDriverAdd
  Wire.write(PWMFrequenceSet);        // set frequence header
  Wire.write(Frequence);              //  send frequence 
  Wire.write(Nothing);              //  need to send this byte as the third byte(no meaning)  
  Wire.endTransmission();    // stop transmitting
}

void MotorDirectionSet(unsigned char Direction)  {     //  Adjust the direction of the motors 0b0000 I4 I3 I2 I1
  Wire.beginTransmission(I2CMotorDriverAdd); // transmit to device I2CMotorDriverAdd
  Wire.write(DirectionSet);        // Direction control header
  Wire.write(Direction);              // send direction control information
  Wire.write(Nothing);              // need to send this byte as the third byte(no meaning)  
  Wire.endTransmission();    // stop transmitting 
}

void MotorDriectionAndSpeedSet(unsigned char Direction,unsigned char MotorSpeedA,unsigned char MotorSpeedB)  {  //you can adjust the driection and speed together
  MotorDirectionSet(Direction);
  MotorSpeedSetAB(MotorSpeedA,MotorSpeedB);  
}

void setup()  {
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(9600);
  delayMicroseconds(10000); //wait for motor driver to initialization
  MotorSpeedSetAB(0,0);    // off=0 full-on= 100
  delay(10);             //this delay needed
  MotorDirectionSet(0b1010);  //0b1010 sets '+' to pos and '-' to negative on both outputs
  pinMode(ledPin, OUTPUT); 
  pinMode(motionPin, INPUT);  // PIR device has: '+' to VCC, 'out' to motionPin, 
                              // and '-' to 10kohm to GND
void loop()  {
  while(1)  {  //superfluous
  // read the value from the slider/pot:
  sensorValue = analogRead(sensorPin); 
  if ((oldSensorVal > (sensorValue + dull)) || (oldSensorVal < (sensorValue - 10)))
    {blinkrate = map(sensorValue,0,1023,0,100);
    oldSensorVal = sensorValue;
    MotorSpeedSetAB(blinkrate,blinkrate);
    countdown = goodnight;
    Serial.print("slider = " );
    Serial.print(sensorValue);
    Serial.print("\t duty-cycle = ");
    Serial.println(blinkrate);
    } 
  // turn the ledPin on
  digitalWrite(ledPin, LOW);  
  // stop the program for <sensorValue> milliseconds:
  delay(100 - blinkrate);          
  // turn the ledPin off:        
  digitalWrite(ledPin, HIGH);   
  // stop the program for for <sensorValue> milliseconds:
  delay(blinkrate);
  countdown -= 100;
  if (countdown < 102)    // unsigned long would loop back to a big positive
    {alert = digitalRead(motionPin);
    if (alert)
      {MotorSpeedSetAB(100,100);
      Serial.println("ALERT!");
      blinkrate = 90;}
    else
      {MotorSpeedSetAB(0,0);
      Serial.print("normal");
      Serial.println(alert);
      blinkrate = 30;}
    countdown = 200;   // but we don't let that happen
    //Serial.println("sleeping");
    }  
  }
}

