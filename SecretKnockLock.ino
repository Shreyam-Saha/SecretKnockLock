

// import the library
#include <Servo.h>
// create an instance of the Servo library
Servo myServo;

const int piezo = A0;      // pin the piezo is attached to
const int switchPin = 2;    // pin the switch is attached to

const int yellowLED = 3;    // pin the yellow LED is attached to
const int greenLED = 4;    // pin the green LED is attached to
const int redLED = 5;   // pin the red LED is attached to

const int threshold=100;
const int rejectValue = 25;
const int avgRejectValue = 15;
const int knockFadeTime=150;
const int lockTurnTime = 650;
const int maxknocks = 10;
const int closedoor = 5;
const int knockComplete = 1200;
//const int lockMotor = 3;

// variable for the piezo value
int knockVal;
// variable for the switch value
int switchVal;

int Secretknock[maxknocks]={50,50,50,50,50,50,0,0,0,0};
int knockReads[maxknocks];
int knockSensorValue = 0;   //Last reading of the knock sensor
int buttonPressed = false;
void listenToSecretKnock();
/*// variables for the high and low limits of the knock value
const int quietKnock = 10;
const int loudKnock = 100;

// variable to indicate if locked or not
boolean locked = false;
// how many valid knocks you've received
  int numberOfKnocks = 0;
*/
void setup() {
  // attach the servo to pin 9
  myServo.attach(9);

  // make the LED pins outputs
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  // set the switch pin as an input
  pinMode(switchPin, INPUT);

  pinMode(closedoor, INPUT);
  
  // start serial communication for debugging
  Serial.begin(9600);

  // turn the green LED on
  digitalWrite(greenLED, HIGH);

  // move the servo to the unlocked position
  myServo.write(0);

  // print status to the Serial Monitor
//  Serial.println("the box is unlocked!");
}

void loop() {

  knockSensorValue=analogRead(piezo);
  if(digitalRead(switchPin) == HIGH){
    buttonPressed=true;
    digitalWrite(redLED,HIGH);
  }
  else{
    buttonPressed=false;
    digitalWrite(redLED,LOW); 
  }
if(knockSensorValue>=threshold){
   listenToSecretKnock();
}
}
void listenToSecretKnock(){
 Serial.println("knock starting");

   myServo.write(0);


  int i = 0;
  // First lets reset the listening array.
  for (i=0;i<maxknocks;i++){
    knockReads[i]=0;
  }
  
  int currentKnockNumber=0;              // Incrementer for the array.
  int startTime=millis();                 // Reference for when this knock started.
  int now=millis();
  int validateKnock();
  digitalWrite(greenLED, LOW);            // we blink the LED for a bit as a visual indicator of the knock.
  if (buttonPressed==true){
     digitalWrite(redLED, LOW);                         // and the red one too if we're programming a new knock.
  }
  delay(knockFadeTime);                                 // wait for this peak to fade before we listen to the next one.
  digitalWrite(greenLED, HIGH);  
  if (buttonPressed==true){
     digitalWrite(redLED, HIGH);                        
  }
  do {
    //listen for the next knock or wait for it to timeout. 
    knockSensorValue = analogRead(piezo);
    if (knockSensorValue >=threshold){                   //got another knock...
      //record the delay time.
      Serial.println("knock.");
      now=millis();
      knockReads[currentKnockNumber] = now-startTime;
      currentKnockNumber ++;                             //increment the counter
      startTime=now;          
      // and reset our timer for the next knock
      digitalWrite(greenLED, LOW);  
      if (buttonPressed==true){
        digitalWrite(redLED, LOW);                       // and the red one too if we're programming a new knock.
      }
      delay(knockFadeTime);                              // again, a little delay to let the knock decay.
      digitalWrite(greenLED, HIGH);
      if (buttonPressed==true){
        digitalWrite(redLED, HIGH);                         
      }
    }

    now=millis();
    
    //did we timeout or run out of knocks?
  } while ((now-startTime < knockComplete) && (currentKnockNumber < maxknocks));
  
  //we've got our knock recorded, lets see if it's valid
  if (buttonPressed==false){             // only if we're not in progrmaing mode.
    if (validateKnock() == true){
      triggerDoorUnlock(); 
    } else {
      Serial.println("Secret knock failed.");
      digitalWrite(greenLED, LOW);      // We didn't unlock, so blink the red LED as visual feedback.
      for (i=0;i<4;i++){          
        digitalWrite(redLED, HIGH);
        delay(100);
        digitalWrite(redLED, LOW);
        delay(100);
      }
      digitalWrite(greenLED, HIGH);
    }
  } else { // if we're in programming mode we still validate the lock, we just don't do anything with the lock
    validateKnock();
    // and we blink the green and red alternately to show that program is complete.
    Serial.println("New lock stored.");
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    for (i=0;i<3;i++){
      delay(100);
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
      delay(100);
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, HIGH);      
    }
  }
}


// Runs the motor (or whatever) to unlock the door.
void triggerDoorUnlock(){
  Serial.println("Door unlocked!");
  int i=0;
  
  // turn the motor on for a bit.
 // digitalWrite(lockMotor, HIGH);
  myServo.write(90);
  digitalWrite(greenLED, HIGH);            // And the green LED too.
  
  delay (lockTurnTime);                    // Wait a bit.
  
 // digitalWrite(lockMotor, LOW);            // Turn the motor off.
 /*  if(switchPin ==LOW){
     myServo.write(0);
     delay(10000);
   */ 
  // Blink the green LED a few times for more visual feedback.
  for (i=0; i < 5; i++){   
      digitalWrite(greenLED, LOW);
      delay(100);
      digitalWrite(greenLED, HIGH);
      delay(100);
  }
   
}

// Sees if our knock matches the secret.
// returns true if it's a good knock, false if it's not.
// todo: break it into smaller functions for readability.
boolean validateKnock(){
  int i=0;
 
  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;               // We use this later to normalize the times.
  
  for (i=0;i<maxknocks;i++){
    if (knockReads[i] > 0){
      currentKnockCount++;
    }
    if (Secretknock[i] > 0){           //todo: precalculate this.
      secretKnockCount++;
    }
    
    if (knockReads[i] > maxKnockInterval){   // collect normalization data while we're looping.
      maxKnockInterval = knockReads[i];
    }
  }
  
  // If we're recording a new knock, save the info and get out of here.
  if (buttonPressed==true){
      for (i=0;i<maxknocks;i++){ // normalize the times
        Secretknock[i]= map(knockReads[i],0, maxKnockInterval, 0, 100); 
      }
      // And flash the lights in the recorded pattern to let us know it's been programmed.
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, LOW);
      delay(1000);
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, HIGH);
      delay(50);
      for (i = 0; i < maxknocks ; i++){
        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, LOW);  
        // only turn it on if there's a delay
        if (Secretknock[i] > 0){                                   
          delay( map(Secretknock[i],0, 100, 0, maxKnockInterval)); // Expand the time back out to what it was.  Roughly. 
          digitalWrite(greenLED, HIGH);
          digitalWrite(redLED, HIGH);
        }
        delay(50);
      }
    return false;   // We don't unlock the door when we are recording a new knock.
  }
  
  if (currentKnockCount != secretKnockCount){
    return false; 
  }
  
  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if you're tempo is a little slow or fast. 
  */
  int totaltimeDifferences=0;
  int timeDiff=0;
  for (i=0;i<maxknocks;i++){ // Normalize the times
    knockReads[i]= map(knockReads[i],0, maxKnockInterval, 0, 100);      
    timeDiff = abs(knockReads[i]-Secretknock[i]);
    if (timeDiff > rejectValue){ // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences/secretKnockCount>avgRejectValue){
    return false; 
  }
  
  return true;
  
}
