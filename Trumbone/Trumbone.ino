#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial mySerial(2, 3); // RX, TX
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

byte note = 0; // The MIDI note value to be played
byte resetMIDI = 4; // Tied to VS1053 Reset line
byte ledPin = 13; // MIDI traffic inidicator
int  instrument = 0; // Default Piano (but we use 57 (Trumbone))

int echo = 6;
int trig = 7;
const int buttonPin = 4;
const int octaveButton = 0;

void setup() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  lcd.begin(16, 2);

  Serial.begin(57600);
  pinMode(buttonPin, INPUT);
  pinMode(octaveButton, INPUT);
  //Setup soft serial for MIDI control
  mySerial.begin(31250);

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0x07, 127); // 0xB0 is channel message, set channel volume to near max (127)
}

/* Each Melody (C,D,E,F,G,A,B,C) */
int sound[3][7] = { { 24,26,28,29,31,33,35 },
  { 36,38,40,41,43,45,47 },
  { 48,50,52,53,55,57,59 }
}; // Ocatave 2, Ocatave 3, Ocatave 4

int i = 0; // For Octavae Button
int distance = 0;
int buttonState = 0; // Melody control button
int buttonState1 =0; // Octave change button

void loop() {
  talkMIDI(0xB0, 0, 0x00); // Default bank GM1
  talkMIDI(0xC0, instrument, 0); // Set instrument number. 0xC0 is a 1 data byte command

  buttonState = digitalRead(buttonPin);
  Serial.print("button: ");
  Serial.println(buttonState);

  buttonState1 = digitalRead(octaveButton);
  Serial.print("octave: ");
  Serial.println(buttonState1);

  int noteNumber = 0;

  if (buttonState == HIGH) { // melody control button High then sound on
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    distance = pulseIn(echo, HIGH) * 17 / 1000;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Distance: ");
    lcd.print(distance);
    lcd.print(" cm");

    if (distance >= 5 && distance <= 30) {
      noteNumber = map(distance, 5, 30, 0, 6);
    }

    lcd.setCursor(0, 1);
    lcd.print(i);

    lcd.print(" Note: ");

    if (noteNumber == 0) {
      lcd.print("C");
    }
    else if (noteNumber == 1) {
      lcd.print("D");
    }
    else if (noteNumber == 2) {
      lcd.print("E");
    }
    else if (noteNumber == 3) {
      lcd.print("F");
    }
    else if (noteNumber == 4) {
      lcd.print("G");
    }
    else if (noteNumber == 5) {
      lcd.print("A");
    }
    else if (noteNumber == 6) {
      lcd.print("B");
    }

    instrument = 57;
    talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command
    noteOn(0, sound[i][noteNumber], 100); // sound on
    delay(1500);
    noteOff(0, sound[i][noteNumber], 0); // sound off
  }

  if (buttonState1 == LOW) { // if button press then octave change
    if (i == 2) {
      i = 0;
    }
    else {
      i++;
    }
  }
}

// Send a MIDI note-on message.  Like pressing a piano key
// channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI((0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI((0x80 | channel), note, release_velocity);
}

// Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if ((cmd & 0xF0) <= 0xB0)
  mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}
