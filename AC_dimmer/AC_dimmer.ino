/*
******************AC Light Control************************
* Changed zero-crossing detection to look for RISING edge rather
* than falling.  (originally it was only chopping the negative half
* of the AC wave form). 
* Also changed the dim_check() to turn on the Triac, leaving it on 
* until the zero_cross_detect() turn's it off.
*/
volatile int i= 0;               // Variable to use as a counter
volatile boolean zero_cross=0;   // Boolean to store a "switch" to tell us if we have crossed zero
int TRIAC_CTRL = 33;              // Output to Opto Triac
int ZERO_CROSS_PIN = 17;          // Input from Optocoupler
volatile int dim = 0;            // Dimming level (0-128)  0 = on, 128 = 0ff
int inc = 1;                     // counting up or down, 1=up, -1=down

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

int freqStep = 75;    // This is the delay-per-brightness step in microseconds.
// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want. 
// 
// The only tricky part is that the chopper circuit chops the AC wave twice per
// cycle, once on the positive half and once at the negative half. This meeans
// the chopping happens at 120Hz for a 60Hz supply or 100Hz for a 50Hz supply. 

// To calculate freqStep you divide the length of one full half-wave of the power
// cycle (in microseconds) by the number of brightness steps. 
//
// (1000000 uS / 100 Hz) / 128 brightness steps ~ 75 uS / brightness step


/************Begin setup**************/
void setup() {  

  pinMode(TRIAC_CTRL, OUTPUT);                          // Set the Triac pin as output
  pinMode(ZERO_CROSS_PIN,INPUT_PULLUP);                 // 
  attachInterrupt(ZERO_CROSS_PIN, zero_cross_detect, RISING);   // Attach an Interupt to Pin 17 (interupt 0) for Zero Cross Detection
  timer = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns  TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns  80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(timer, &dim_check, true); // edge (not level) triggered 
  timerAlarmWrite(timer, freqStep, true); //  autoreload true
  timerAlarmEnable(timer); // enable  
  
}
/************function to be fired at the zero crossing*************/
void zero_cross_detect() {  
 
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
  digitalWrite(TRIAC_CTRL, LOW);       // turn off TRIAC (and AC)

}                                 

/***********function turn on the TRIAC at the appropriate time*************/
void dim_check() {   
  portENTER_CRITICAL_ISR(&timerMux); 
          
  if(zero_cross == true) {              
    if(i>=dim) {                     
      digitalWrite(TRIAC_CTRL, HIGH); // turn on light       
      i=0;  // reset time step counter                         
      zero_cross = false; //reset zero cross detection
    } 
    else {
      i++; // increment time step counter     
                
    }                                
  } 
   portEXIT_CRITICAL_ISR(&timerMux);                                 
}                                   

void loop() { 
                      
  dim+=inc;

  if((dim>=128) || (dim<=0))
   { delay(1000); 
     inc*=-1;    
    
   }  
  delay(70);
}
