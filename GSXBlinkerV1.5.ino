

//Created : 2015-03-13
//Author : Martin Knudsen
//Purpose : To control turn indicators on motorbike

//Version / Changes: 
//0.1 / Draft
//1.0 / Version for commissioning
//1.1 / PWM for blinker added
//1.2 / PWM for blinker removed
//1.3 / Pin 13 added for control purposes
//1.4 / Lane changing function added
//1.5 / Blink interval changed from ON:750, OFF:750 to -> ON:600, OFF:500


// ---------------------------------------------------------------------------//
//				 Inclusiions
// ---------------------------------------------------------------------------//


// ---------------------------------------------------------------------------//
//				 Declerations I/O
// ---------------------------------------------------------------------------//

// Setup of I/O
// Digital
int OutLeft = 9;
int OutRight = 10;
int SwitchLeft = 12;
int SwitchRight = 4;
int OutControlLed = 13;

// Digital I/O
bool bLeft;
bool bRight;

// ---------------------------------------------------------------------------//
//				 Declerations other var
// ---------------------------------------------------------------------------//


// EM01
int iEM01Cmd = 0;
int iEM01Seq = 0;
int iEM01SeqLastStep = 0;
bool bEM01SeqStep = false;

unsigned long iEM01_SeqTimer001;
unsigned long iEM01_SeqTimerPre001;
bool iEM01_SeqTimerDone001;

int iBlinkerOn = 600;
int iBlinkerOff = 500;
int iPWMBlinker = 150;
int iLaneChange = 0;

bool bLeftMemory;
bool bRightMemory;

unsigned long iControlLED_on;
unsigned long iControlLED_off;
bool bControlLED_on;
bool bControlLED_off;

// System variables
bool bFirstScan = true;
bool bFirstScanAux = false;
unsigned long iLastScan;
unsigned long iMilliSec;
unsigned long iMilliSecLast;


// ---------------------------------------------------------------------------//
//				    Subroutines
// ---------------------------------------------------------------------------//

// ---------------------------------------------------------------------------//
//				    System functions
// ---------------------------------------------------------------------------//
void fncSystem(){
/* Associated system variables
// System variables
bool bFirstScan = true;
bool bFirstScanAux = false;
unsigned long iLastScan;
unsigned long iMilliSec;
unsigned long iMilliSecLast;
*/
 
  
// First scan
  if ( bFirstScanAux == true )
    {
     	bFirstScan = false; 
    }
  	bFirstScanAux = true;
  

// Timer tags
	// Get the milliseconds  
  	iMilliSec = millis();
  
 	// Find if normal operation or rollover 
  	if (iMilliSec >=  iLastScan) // Normal operation
	{
		iLastScan = iMilliSec - iMilliSecLast; 
	}
	else // Rollover
	{
		iLastScan = iMilliSec  + (4294967295 - iMilliSecLast);
		Serial.print("Rollover\r\n");
    }
	// Save last millisec value
	iMilliSecLast = iMilliSec;   
 
  
}


// ---------------------------------------------------------------------------//
//				    Simple timer 
// ---------------------------------------------------------------------------// 

bool SimpleTimer(bool bStart, unsigned long &iTimerVal, unsigned long iTime) {

bool bTimerDone = false;
  
  
// Start timer
if ( bStart and iTimerVal <= iTime)
{
  iTimerVal = iTimerVal + iLastScan; 
}
  
if  (!bStart)
{
  iTimerVal = 0;
}
  
// Return result of timer
if ( bStart and ( iTimerVal >= iTime)  and (iTimerVal > 0))
{
  bTimerDone = true;
}
else
{
  bTimerDone = false;  
}  
  
return bTimerDone;
}


// ---------------------------------------------------------------------------//
//				EM01 - Blink control 
// ---------------------------------------------------------------------------//
void EM01(int iCMD, int &iSeq, int &iSeqLastStep, bool &bSeqStep, unsigned long &iSeqTimer001, unsigned long &iSeqTimerPre001, bool &bSeqTimerDone001, int imSecOn, int imSecOff, int &iLaneChangeBlinks) {

  
//  ------------- Declerations ------------- 


//  ------------- Initialize -------------

  
//  ------------- State machine -------------  

  // Multitrans
  if (iCMD == 0 and iLaneChangeBlinks >= 4)
  {
    iSeq = 0;    
  }


  // Seq
    switch (iSeq) {

      case 0: // Idle
      	// Init action
      	if (bSeqStep) Serial.println("EM01 - Idle");  
      	// Step
        iLaneChangeBlinks = 0;
      	if (iCMD == 1) iSeq = 10; // Jump to step 10 - On
      	break;

      case 10: // On
        // Init action
      	if (bSeqStep) 
        {
          Serial.println("EM01 - On");
          iSeqTimerPre001 = imSecOn;
        }
        // Step
      	if (bSeqTimerDone001 and !bSeqStep)  
        {
          iSeq = 12; // Jump to step 12 - Off
        }
      break;

      case 12: // Off
        // Init action
      	if (bSeqStep) 
        {
          Serial.println("EM01 - Off");
          iSeqTimerPre001 = imSecOff;
          iLaneChangeBlinks = iLaneChangeBlinks + 1;
        }
        // Step
      	if (bSeqTimerDone001 and !bSeqStep)  
        {
          iSeq = 10; // Jump to step 10 - On
        }
      break;    
      
      default: 
        iSeq = 0;
  }
    
//  ------------- Change step statuses -------------
  bSeqStep = iSeqLastStep != iSeq;
  iSeqLastStep = iSeq;

//  ------------- State machine timers -------------
// Wait timer
  
bSeqTimerDone001 =  SimpleTimer((iSeq == 10 or iSeq == 12) and !bSeqStep, iSeqTimer001, iSeqTimerPre001);
 
}

// ---------------------------------------------------------------------------//
//				      Setup
// ---------------------------------------------------------------------------//

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(OutLeft, OUTPUT);
  pinMode(OutRight, OUTPUT);
  pinMode(SwitchLeft, INPUT);  
  pinMode(SwitchRight, INPUT);  
  pinMode(OutControlLed, OUTPUT);  
  
  // Serial begin
  Serial.begin(9600);      // open the serial port at 9600 bps: 

  
}

// ---------------------------------------------------------------------------//
//				   Main Loop
// ---------------------------------------------------------------------------//

// the loop routine runs over and over again forever:
void loop() {

//  ------------- SYSTEM FUNCTIONS -------------
  fncSystem();
  
  
//  ------------- Read  DI ------------- 
  bLeft = digitalRead(SwitchLeft);  
  bRight = digitalRead(SwitchRight);

  
//  ------------ EM01 - Commands -------------

  if (bLeft)
  {
    bLeftMemory = HIGH;
  }
  if (bRight or iEM01Seq == 0)
  {
    bLeftMemory = LOW;    
  }


  if (bRight)
  {
    bRightMemory = HIGH;
  }
  if (bLeft or iEM01Seq == 0)
  {
    bRightMemory = LOW;    
  }


  if(bLeft or bRight) 
    iEM01Cmd = 1;
  else
    iEM01Cmd = 0;
  
  
 //  ------------ EM01 - Flasher -------------iLaneChange
  EM01(iEM01Cmd, iEM01Seq, iEM01SeqLastStep, bEM01SeqStep, iEM01_SeqTimer001, iEM01_SeqTimerPre001, iEM01_SeqTimerDone001, iBlinkerOn, iBlinkerOff, iLaneChange);

// Outputs 
digitalWrite(OutLeft, bLeftMemory and iEM01Seq == 10);   // turn the LED on (HIGH is the voltage level)  	
digitalWrite(OutRight, bRightMemory and iEM01Seq == 10);   // turn the LED on (HIGH is the voltage level)

 
// Control flasher 
bControlLED_on = SimpleTimer(!bControlLED_off, iControlLED_on, 500);
bControlLED_off = SimpleTimer(bControlLED_on, iControlLED_off, 500);    
digitalWrite(OutControlLed, iEM01Seq == 10);   // turn the LED on (HIGH is the voltage level)

     
delay(10);               // wait for a second

}

