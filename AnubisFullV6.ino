//Richard Moore
//September 21, 2013

//This version uses interrupts to ensure that the encoder is always read properly.  

//This version is set up to use potentiometers run to analog inputs A0 and A1 instead
//of encoders.  

#include <SPI.h>

//static const int taps[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10, 11, 12, 14, 15, 17, 19, 21, 23, 26, 28, 31, 33, 36, 38, 41, 43, 46, 48, 51, 53, 56, 58, 60, 62, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 84, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 102, 104, 105, 106, 108, 109, 110, 112, 113, 114, 115, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 127, 127};
//This next table is the new one I got from measuring the output with a volt meter:
static const int taps[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 10, 15, 19, 24, 28, 32, 35, 39, 42, 46, 49, 51, 54, 57, 59, 61, 64, 66, 68, 70, 72, 73, 75, 77, 78, 80, 81, 83, 84, 86, 87, 88, 89, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 100, 101, 102, 103, 104, 104, 105, 106, 106, 107, 108, 108, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119, 119, 120, 120, 121, 121, 121, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 126, 126, 126, 126, 127, 127};
int program = 1; //ranges 1 to max program
int value = 1; //ranges 0 to 255, actually 0-127 with new digipots
int value1 = 1;
int value2 = 1;
float pi = 3.14159;
int clock = 2;
//These are for the value potentiometers:
int ValPot1 = 0;
int ValPot2 = 1;
//These are for the digital outputs and inputs:
int NeckDigi = 3;
int MidDigi = 3;//4;//For the dual u/d digipots NeckDigi and MidDigi share the same line.
int BridgeDigi = 5;
int NeckSw = 6;
int MidSw = 7;
int BridgeSw = 8;
int ProgEncA = 9;
int ProgEncB = 10;
int ValEncA = 11;
int ValEncB = 12;
//These won't really work with the dual digipots:
int NeckDigiCS = 0;
int MidDigiCS = 1;
int BridgeDigiCS = 13;
//
int ProgEncACurr = LOW;
int ProgEncALast = LOW;
int ValEncACurr = LOW;
int ValEncALast = LOW;
int PUind = 1;
int c1 = 0;
int c2 = 0;
int c3 = 0;
int Val1 = 0;//0;
int Val2 = 0;//0;
int Val3 = 0;//0;
float t1 = 0;
int prevInd = 1;
int OldD1 = 127;
int OldD2 = 127;
int OldD3 = 127;
int NeckSwPol = HIGH;
int MidSwPol = HIGH;
int BridgeSwPol = HIGH;

//Gotta use these instead (for dual digipots):
int DACsel = 0;//LOW for Neck, High for Middle
int DigiChip1CS = 1;
int DigiChip2CS = 13;

int V1table[] = {126, 96, 64, 32, 0, 0, 0, 0, 0};
int V2table[] = {0, 32, 64, 96, 126, 96, 64, 32, 0};
int V3table[] = {0, 0, 0, 0, 0, 32, 64, 96, 128};

//Sept 2: For these u/d digipots, I may need to do an initialization to make sure
//that the initial position of the digipots is known after a new program has been 
//uploaded or the power has been cycled, etc.  

void setup() {
  //clock:
  pinMode(clock, OUTPUT);
  
  //digipot 1 (neck):
  pinMode(NeckDigi, OUTPUT);
  
  //digipot 2 (middle):
  pinMode(MidDigi, OUTPUT);
  
  //digipot 3 (bridge):
  pinMode(BridgeDigi, OUTPUT);
  
  //Chip Select for the neck digipot:
  pinMode(NeckDigiCS, OUTPUT);
  
  //Chip select for the middle digipot:
  pinMode(MidDigiCS, OUTPUT);
  
  //Chip select for the bridge digipot:
  pinMode(BridgeDigiCS, OUTPUT);
  
  //Neck polarity switch:
  pinMode(NeckSw, OUTPUT);
  
  //Middle polarity switch:
  pinMode(MidSw, OUTPUT);
  
  //Bridge polarity switch:
  pinMode(BridgeSw, OUTPUT);
  
  //encoder 1 channel A (program):
  pinMode(ProgEncA, INPUT);
  
  //encoder 1 channel B (program);
  pinMode(ProgEncB, INPUT);
  
  //encoder 2 channel A (value):
  pinMode(ValEncA, INPUT);
  
  //encoder 2 channel B (value):
  pinMode(ValEncB, INPUT);
  
  //Make interrupts for the encoder leads:
  attachInterrupt(num,func,mode);
  attachInterrupt(num,func,mode);
  //Right now the leads to the program encoder are 9 and 10.  
  //I'll need to move those leads to inputs 2 and 3.
  //The current items at pins 2 and 3 are the clock and the NeckDigi and BridgeDigi.
  //I need to figure out how the functions for the interrupts need to work.
  //I think that I'll need to pass at least one pointer for a state.  
  
  //I'm not sure if this next line is needed or not:
  //Serial.begin(9600);
  
  //zeroize the digipots:
  WriteOutput(Val1, Val2, Val3, NeckDigi, MidDigi, BridgeDigi, &OldD1, &OldD2, &OldD3, NeckSwPol, MidSwPol, BridgeSwPol, NeckSw, MidSw, BridgeSw);
}

void loop() {
  //Increment/decrement program if needed:
  ProgEncACurr = digitalRead(ProgEncA);
   if ((ProgEncALast == LOW) && (ProgEncACurr == HIGH)) {
     if (digitalRead(ProgEncB) == LOW) {
       program--;
     } else {
       program++;
     }
     //Serial.print (program);
     //Serial.print ("/");
   } 
   ProgEncALast = ProgEncACurr;
   //Stay in bounds:
   program = constrain(program,1,4);
  
   //Increment/decrement value if needed:
   //ValEncACurr = digitalRead(ValEncA);
   // if ((ValEncALast == LOW) && (ValEncACurr == HIGH)) {
   //   if (digitalRead(ValEncB) == LOW) {
   //     value--;
   //   } else {
   //     value++;
   //   }
   //   //Serial.print (value);
   //   //Serial.print ("/");
   // } 
   // ValEncALast = ValEncACurr;
   //Stay in bounds:
   //value = constrain(value, 1, 127);//255);
   value1 = analogRead(ValPot1);
   value2 = analogRead(ValPot2);
   value1 = (value1/32)*32;
   value2 = (value2/32)*32;
  //Run the appropriate program:
  switch (program) {
    //case 1:
    //  Val1 = 0;
    //  Val2 = 15;
    //  Val3 = 0;
    //  WriteOutput(Val1, Val2, Val3, NeckDigi, MidDigi, BridgeDigi, &OldD1, &OldD2, &OldD3, NeckSwPol, MidSwPol, BridgeSwPol, NeckSw, MidSw, BridgeSw);
    //  break;
    case 1:
      //Continuous fade from neck to bridge:
      //Right now value1 can be 0-1023, but the tables are only about 8 terms long.
      Val1 = V1table[value1];
      Val2 = V2table[value1];
      Val3 = V3table[value1];
      WriteOutput(Val1, Val2, Val3, NeckDigi, MidDigi, BridgeDigi, &OldD1, &OldD2, &OldD3, NeckSwPol, MidSwPol, BridgeSwPol, NeckSw, MidSw, BridgeSw);
      break;
    case 2:
      //Oscillating pickup position:
      //Val1 = int(127*0.5*(1+sin(2*pi*millis()/(value*100))));
      //Val2 = int(127*0.5*(1+sin(2*pi/3+2*pi*millis()/(value*100))));
      //Val3 = int(127*0.5*(1+sin(2*2*pi/3+2*pi*millis()/(value*100))));
      //Val1 = int(127*0.5*(1+sin(2*pi*millis()/(value*100))));
      //Val2 = int(127*0.5*(1+sin(2*pi/3+2*pi*millis()/(value*100))));
      //Val3 = int(127*0.5*(1+sin(2*2*pi/3+2*pi*millis()/(value*100))));
      //This uses the analog potentiometers and has freq. scaled from 0.1Hz to 25Hz or so.
      Val1 = int(127*(1*((1023-float(value2))/1023)+(float(value2)/1023)*0.5*(1+sin(2*pi*0.001*millis()*(0.1*pow(2,float(value1)/128))))));
      Val1 = int(127*(1*((1023-float(value2))/1023)+(float(value2)/1023)*0.5*(1+sin(2*pi/3+2*pi*0.001*millis()*(0.1*pow(2,float(value1)/128))))));
      Val1 = int(127*(1*((1023-float(value2))/1023)+(float(value2)/1023)*0.5*(1+sin(2*2*pi/3+2*pi*0.001*millis()*(0.1*pow(2,float(value1)/128))))));
      
      WriteOutput(Val1, Val2, Val3, NeckDigi, MidDigi, BridgeDigi, &OldD1, &OldD2, &OldD3, NeckSwPol, MidSwPol, BridgeSwPol, NeckSw, MidSw, BridgeSw);
      break;
    case 3:
      //Tremolo, all pickups in unison:
      //Val1 = int(127*0.5*(1+sin(2*pi*millis()/(value1*100))));
      //Val2 = int(127*0.5*(1+sin(2*pi*millis()/(value1*100))));
      //Val3 = int(127*0.5*(1+sin(2*pi*millis()/(value1*100))));
      //This uses the analog potentiometers and has freq. scaled from 0.1Hz to 25Hz or so.
      Val1 = int(127*(1*((1023-float(value2))/1023)+(float(value2)/1023)*0.5*(1+sin(2*pi*0.001*millis()*(0.1*pow(2,float(value1)/128))))));
      Val2 = int(127*(1*((1023-float(value2))/1023)+(float(value2)/1023)*0.5*(1+sin(2*pi*0.001*millis()*(0.1*pow(2,float(value1)/128))))));
      Val3 = int(127*(1*((1023-float(value2))/1023)+(float(value2)/1023)*0.5*(1+sin(2*pi*0.001*millis()*(0.1*pow(2,float(value1)/128))))));
      
      WriteOutput(Val1, Val2, Val3, NeckDigi, MidDigi, BridgeDigi, &OldD1, &OldD2, &OldD3, NeckSwPol, MidSwPol, BridgeSwPol, NeckSw, MidSw, BridgeSw);
      break;
    case 4:
      //Two things are still wrong with this program:
      //1. The change to the new value is immediate, causing a clicking sound.
      //I should let the values transition more gradually.
      //2. With just 3 pickups, often the pickup selected doesn't change.
      //Maybe when one pickup is selected I should have the program
      //randomly selected between the two remaining pickups.
      //Or, I could allow pickup mixing.  
      //if (millis()>(t1+float(value*10)))
      if (millis()>(t1+float(1000*1/(0.1*pow(2,float(value1)/128)))))
      {
      PUind = int(random(0.5,3.49));
      while ((prevInd==PUind)|((PUind!=1)&(PUind!=2)&(PUind!=3))) {
        PUind = int(random(0.5,3.49));
      }
      prevInd = PUind;
      t1 = millis();
      switch (PUind) {
        case 1:
          PUind = 2;
          break;
        case 2:
          PUind = 3;
          break;
        case 3:
          PUind = 1;
          break;
      }
      switch (PUind) {
        case 1:
          Val1 = 127;//255;
          Val2 = 1;
          Val3 = 1;
          break;
        case 2:
          Val1 = 1;
          Val2 = 127;//255;
          Val3 = 1;
          break;
        case 3:
          Val1 = 1;
          Val2 = 1;
          Val3 = 127;//255;
          break;
      }
      WriteOutput(Val1, Val2, Val3, NeckDigi, MidDigi, BridgeDigi, &OldD1, &OldD2, &OldD3, NeckSwPol, MidSwPol, BridgeSwPol, NeckSw, MidSw, BridgeSw);
      //t1 = millis();
      }
      break;
  }
  //Does anything else need to happen before we check encoders again?
}

///////////////////////////////////////////////////
//Hmmmmmmmmm 15 arguments?  Kind ridiculous.  
int WriteOutput(int d1, int d2, int d3, int Loc1, int Loc2, int Loc3, int *OldD1, int *OldD2, int *OldD3, int NeckSwPol, int MidSwPol, int BridgeSwPol, int Loc4, int Loc5, int Loc6){
  //
  
  //When the pot values jump by large amounts, there is an unpleasant
  //clicking noise that results.  I need to implement a slew limiter to 
  //avoid that noise.  Right now there's no memory of the previous value,
  //so I'll need to build that in as well.
  
  //static const int taps[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10, 11, 12, 14, 15, 17, 19, 21, 23, 26, 28, 31, 33, 36, 38, 41, 43, 46, 48, 51, 53, 56, 58, 60, 62, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 84, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 102, 104, 105, 106, 108, 109, 110, 112, 113, 114, 115, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 127, 127};
  //This next table is the new one I got from measuring the output with a volt meter:
  static const int taps[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 10, 15, 19, 24, 28, 32, 35, 39, 42, 46, 49, 51, 54, 57, 59, 61, 64, 66, 68, 70, 72, 73, 75, 77, 78, 80, 81, 83, 84, 86, 87, 88, 89, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 100, 101, 102, 103, 104, 104, 105, 106, 106, 107, 108, 108, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119, 119, 120, 120, 121, 121, 121, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 126, 126, 126, 126, 127, 127};
  
  int w1 = *OldD1;
  int w2 = *OldD2;
  int w3 = *OldD3;
  
  double Ri = 1000000;  //Input impedance of the amplifier
  double Ro = 10000;    //Output impedance of the pickup itself
  //double Rp = 1000000-d1*1000000/127;        //This will be 1M-Rg
  //double Rg = 0;        //This is the portion of the digipot between wiper and ground.
  double k = 1/127;     //This is the constant that relates dsp levels to normalized output levels.
  
  //taps[] is a lookup table to take the place of the polynomial fits above.  
  d1 = int(taps[d1]);
  d2 = int(taps[d2]);
  d3 = int(taps[d3]);
  
  //Now, for each digipot loop up or down the correct number of times 
  //to get from the previous value to the new value.  This means that 
  //I need to start storing the old values some way.  
  
  //While one of the pots still hasn't reached it's value, continue iterating.
  
  //Also, I need to put in plumbing for the analog switches.  Right now I 
  //don't have variables for their states or anything.  
  while ((w1!=d1)|(w2!=d2)|(w3!=d3)) {
    
    //Neck Digipot
    if (w1<d1) {
      w1 = w1 + 1;
      //Set u/d pin up
      digitalWrite(NeckDigi, HIGH);
      //digitalWrite(NeckDigiCS, LOW);
      digitalWrite(DACsel, LOW);
      digitalWrite(DigiChip1CS, LOW);
    } else if (w1>d1) {
      w1 = w1 - 1;
      //Set u/d pin down
      digitalWrite(NeckDigi, LOW);
      //digitalWrite(NeckDigiCS, LOW);
      digitalWrite(DACsel, LOW);
      digitalWrite(DigiChip1CS, LOW);
    } else {
      //digitalWrite(NeckDigiCS, HIGH);
      //digitalWrite(DACsel, LOW);
      digitalWrite(DigiChip1CS, HIGH);
    }
    
    //Now do the clock
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
    digitalWrite(DigiChip1CS, HIGH);
    
    //Middle Digipot
    if (w2<d2) {
      w2 = w2 + 1;
      //Set u/d pin up
      digitalWrite(MidDigi, HIGH);
      //digitalWrite(MidDigiCS, LOW);
      digitalWrite(DACsel, HIGH);
      digitalWrite(DigiChip1CS, LOW);
    } else if (w2>d2) {
      w2 = w2 - 1;
      //Set u/d pin down
      digitalWrite(MidDigi, LOW);
      //digitalWrite(MidDigiCS, LOW);
      digitalWrite(DACsel, HIGH);
      digitalWrite(DigiChip1CS, LOW);
    } else {
      //digitalWrite(MidDigiCS, HIGH);
      //digitalWrite(DACsel, LOW);
      digitalWrite(DigiChip1CS, HIGH);
    }
    
    //Now do the clock
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
    digitalWrite(DigiChip1CS, HIGH);
    
    //Bridge Digipot
    if (w3<d3) {
      w3 = w3 + 1;
      //Set u/d pin up
      digitalWrite(BridgeDigi, HIGH);
      //digitalWrite(BridgeDigiCS, LOW);
      digitalWrite(DigiChip2CS, LOW);
    } else if (w3>d3) {
      w3 = w3 - 1;
      //Set u/d pin down
      digitalWrite(BridgeDigi, LOW);
      //digitalWrite(BridgeDigiCS, LOW);
      digitalWrite(DigiChip2CS, LOW);
    } else {
      //digitalWrite(BridgeDigiCS, HIGH);
      digitalWrite(DigiChip2CS, HIGH);
    }
    
    //Now do the clock
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
    digitalWrite(DigiChip2CS, HIGH);
    
  }
  //Now write the switch polarities:
  digitalWrite(Loc4, NeckSwPol);
  digitalWrite(Loc5, MidSwPol);
  digitalWrite(Loc6, BridgeSwPol);
  
  //Write the current d1, d2, d3 values to the old variables:
  *OldD1 = d1;
  *OldD2 = d2;
  *OldD3 = d3;
}

