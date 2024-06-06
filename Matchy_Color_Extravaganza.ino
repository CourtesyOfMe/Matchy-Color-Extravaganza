#include <Adafruit_CircuitPlayground.h>

int skillLevel = 1;
int DEBOUNCE = 20;
int sequenceLength = 0;
int currentStep = 0;


void chooseSkillLevel() {
  while (!CircuitPlayground.rightButton()) {
    if (CircuitPlayground.leftButton()) {
      skillLevel = skillLevel + 1;
      if (skillLevel > 4) skillLevel = 1;
      
      CircuitPlayground.clearPixels();
      for (int p=0; p<skillLevel; p++) {
        CircuitPlayground.setPixelColor(p, 0xFFFFFF);
      }
      
      delay(DEBOUNCE); 
    }
  }
}



void newGame() {
  // Set sequence length based on skill level
  switch (skillLevel) {
    case 1:
      sequenceLength = 8;
      break;
    case 2:
      sequenceLength = 14;
      break;
    case 3:
      sequenceLength = 20;
      break;
    case 4:
      sequenceLength = 31;
      break;
  }

  // Populate the game sequence
  for (int i=0; i<sequenceLength; i++) {
    simonSequence[i] = random(4);
  }

  currentStep = 1;
}



void indicateButton(uint8_t b, uint16_t duration) {
  CircuitPlayground.clearPixels();
  for (int p=0; p<3; p++) {
    CircuitPlayground.setPixelColor(simonButton[b].pixel[p], simonButton[b].color);
  }
  CircuitPlayground.playTone(simonButton[b].freq, duration);
  CircuitPlayground.clearPixels();
}



void showSequence() {
  // Set tone playback duration based on current sequence length
  uint16_t toneDuration;
  if (currentStep<=5) {
    toneDuration = 420;
  } else if (currentStep<=13) {
    toneDuration = 320;
  } else {
    toneDuration = 220;
  }

  // Play back sequence up to current step
  for (int i=0; i<currentStep; i++) {
    delay(50);
    indicateButton(simonSequence[i], toneDuration);
  }
}



uint8_t getButtonPress() {
  for (int b=0; b<4; b++) {
    for (int p=0; p<2; p++) {
      if (CircuitPlayground.readCap(simonButton[b].capPad[p]) > CAP_THRESHOLD) {
        indicateButton(b, DEBOUNCE);
        return b;
      }
    }
  }
  return NO_BUTTON;
}



void gameLost(int b) {
  // Show button that should have been pressed
  for (int p=0; p<3; p++) {
    CircuitPlayground.setPixelColor(simonButton[b].pixel[p], simonButton[b].color);
  }

  // Play sad sound :(
  CircuitPlayground.playTone(FAILURE_TONE, 1500);
 
  // And just sit here until reset
  while (true) {}
}



void gameWon() {
  // Play 'razz' special victory signal 
  for (int i=0; i<3; i++) {
    indicateButton(3, 100);  // RED
    indicateButton(1, 100);  // YELLOW
    indicateButton(2, 100);  // BLUE
    indicateButton(0, 100);  // GREEN
  }
  indicateButton(3, 100);  // RED
  indicateButton(1, 100);  // YELLOW

  // Change tones to failure tone
  for (int b=0; b<4; b++) simonButton[b].freq = FAILURE_TONE;

  // Continue for another 0.8 seconds
  for (int i=0; i<2; i++) {
    indicateButton(2, 100);  // BLUE
    indicateButton(0, 100);  // GREEN
    indicateButton(3, 100);  // RED
    indicateButton(1, 100);  // YELLOW
  }

  // Change tones to silence
  for (int b=0; b<4; b++) simonButton[b].freq = 0;

  // Loop lights forever
  while (true) {
    indicateButton(2, 100);  // BLUE
    indicateButton(0, 100);  // GREEN
    indicateButton(3, 100);  // RED
    indicateButton(1, 100);  // YELLOW
  }
}



void setup() {
  CircuitPlayground.begin();

  skillLevel = 1;
  CircuitPlayground.clearPixels();
  CircuitPlayground.setPixelColor(0, 0xFFFFFF);
  chooseSkillLevel();

  randomSeed(millis());

  newGame();
}


void loop() {
  // Show sequence up to current step
  showSequence();

  // Read player button presses
  for (int s=0; s<currentStep; s++) {
    startGuessTime = millis();
    guess = NO_BUTTON;
    while ((millis() - startGuessTime < GUESS_TIMEOUT) && (guess==NO_BUTTON)) {
      guess = getButtonPress();           
    }
    if (guess != simonSequence[s]) {
      gameLost(simonSequence[s]);
    }
  }
  currentStep++;
  if (currentStep > sequenceLength) {
    delay(SEQUENCE_DELAY);
    gameWon();
  }
  delay(SEQUENCE_DELAY);
}