#include <Wire.h>

// 1   a   f   2   3   b
// o   o   o   o   o   o
// 2   4   6   8   10  12
//
// 1   3   5   7   9   11
// o   o   o   o   o   o
// e   d   dp  c   g   4


//ATMEGA328 SCHEMA

//     RESET     1 o  o 28   A5
//     Pin 0     2 o  o 27   A4
//     Pin 1     3 o  o 26   A3
//     Pin 2     4 o  o 25   A2
//     Pin 3     5 o  o 24   A1
//     Pin 4     6 o  o 23   A0
//     VCC       7 o  o 22   GND
//     GND       8 o  o 21   AREF
//     CRYST     9 o  o 20   VCC
//     CRYST    10 o  o 19   Pin 13
//     Pin 5    11 o  o 18   Pin 12
//     Pin 6    12 o  o 17   Pin 11
//     Pin 7    13 o  o 16   Pin 10
//     Pin 8    14 o  o 15   Pin 9

#define LINE_1 9  //display 2
#define LINE_2 10 //display 8
#define LINE_3 A0 //display 10
#define LINE_4 A1  //display 11

#define SEG_A 6 //display 4
#define SEG_B 8 //display 12
#define SEG_C 1 //display 7
#define SEG_D 12 //display 3
#define SEG_E 13 //display 1
#define SEG_F 4  //display 6
#define SEG_G 7 //display 9
#define SEG_DP A3 //display 5



#define SEG_ON LOW
#define SEG_OFF HIGH

#define LINE_ON HIGH
#define LINE_OFF LOW

#define bright 150000

#define DEBUG false


int migliaia = 0;
int centinaia = 0;
int decine = 0;
int unita = 0;
//int actualNumber = 0;
int tickCount = 0;

int ore = 0;
int minuti = 0;
int secondi = 0;


void setup() {

  //effettuo un test dei segmenti
  if (!DEBUG) {
    pinMode(SEG_A, OUTPUT);
    pinMode(SEG_B, OUTPUT);
    pinMode(SEG_C, OUTPUT);
    pinMode(SEG_D, OUTPUT);
    pinMode(SEG_E, OUTPUT);
    pinMode(SEG_F, OUTPUT);
    pinMode(SEG_G, OUTPUT);
    pinMode(SEG_DP, OUTPUT);

    pinMode(LINE_1, OUTPUT);
    pinMode(LINE_2, OUTPUT);
    pinMode(LINE_3, OUTPUT);
    pinMode(LINE_4, OUTPUT);
  }
  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1 = 0;  //initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;    // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10, CS11 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();

  //attivato sleep mode

  //  TCCR2B = (1<<CS22)|(1<<CS20); //Set CLK/128 or overflow interrupt every 1s



  if (DEBUG) {
    Serial.begin(57600);           // start serial for output
    Serial.println("Start");
  }
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event

}


void showNumber(int numero) {
  //divido il numero
  if (numero >= 999) {
    migliaia = numero / 1000;
    numero = numero % 1000;

  } else {
    migliaia = 0;
  }

  if (numero >= 99) {
    centinaia = numero / 100;
    numero = numero % 100;
  } else {
    centinaia = 0;
  }

  if (numero >= 10) {
    decine = numero / 10;
    numero = numero % 10;
  } else {
    decine = 0;
  }
  if (numero < 10) {
    unita = numero;
  } else {
    unita = 0;
  }

}

void singleDisplayPilot(int number) {
  allSegOff();
  switch (number) {
    case 0:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_E, SEG_ON);
      digitalWrite(SEG_F, SEG_ON);
      break;

    case 1:
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      break;

    case 2:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_E, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 3:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 4:
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_F, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 5:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_F, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 6:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_E, SEG_ON);
      digitalWrite(SEG_F, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 7:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      break;

    case 8:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_E, SEG_ON);
      digitalWrite(SEG_F, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 9:
      digitalWrite(SEG_A, SEG_ON);
      digitalWrite(SEG_B, SEG_ON);
      digitalWrite(SEG_C, SEG_ON);
      digitalWrite(SEG_D, SEG_ON);
      digitalWrite(SEG_F, SEG_ON);
      digitalWrite(SEG_G, SEG_ON);
      break;

    case 10:
      digitalWrite(SEG_A, SEG_OFF);
      digitalWrite(SEG_B, SEG_OFF);
      digitalWrite(SEG_C, SEG_OFF);
      digitalWrite(SEG_D, SEG_OFF);
      digitalWrite(SEG_E, SEG_OFF);
      digitalWrite(SEG_F, SEG_OFF);
      digitalWrite(SEG_G, SEG_OFF);
      break;
    case 11:
      digitalWrite(SEG_A, SEG_OFF);
      digitalWrite(SEG_B, SEG_OFF);
      digitalWrite(SEG_C, SEG_OFF);
      digitalWrite(SEG_D, SEG_OFF);
      digitalWrite(SEG_E, SEG_OFF);
      digitalWrite(SEG_F, SEG_OFF);
      digitalWrite(SEG_G, SEG_ON);
      break;
  }

}

void allLineOff() {
  digitalWrite(LINE_1, LINE_OFF);
  digitalWrite(LINE_2, LINE_OFF);
  digitalWrite(LINE_3, LINE_OFF);
  digitalWrite(LINE_4, LINE_OFF);

}

void allSegOff() {
  digitalWrite(SEG_A, SEG_OFF);
  digitalWrite(SEG_B, SEG_OFF);
  digitalWrite(SEG_C, SEG_OFF);
  digitalWrite(SEG_D, SEG_OFF);
  digitalWrite(SEG_E, SEG_OFF);
  digitalWrite(SEG_F, SEG_OFF);
  digitalWrite(SEG_G, SEG_OFF);
  digitalWrite(SEG_DP, SEG_OFF);
}

void allOff() {
  allSegOff();
  allLineOff();
}

void addSecond() {
  secondi++;
  if (secondi == 60) {
    secondi = 0;
    minuti++;
  }
  if (minuti == 60) {
    minuti = 0;
    ore++;
  }
  if (ore == 24) {
    ore = 0;

  }

  showNumber(ore * 100 + minuti);


}

ISR(TIMER1_COMPA_vect) {
  //prescaler settato per frequenza 1Hz con oscillatore 16MHz

  addSecond();

}

void loop() {
  if (!DEBUG) {
    allOff();

    //prima cifra
    digitalWrite(LINE_1, LINE_ON);
    singleDisplayPilot(migliaia);
    digitalWrite(SEG_DP, SEG_OFF);
    delayMicroseconds(bright);
    allOff();
    digitalWrite(LINE_2, LINE_ON);
    singleDisplayPilot(centinaia);
    delayMicroseconds(bright);
    allOff();

    digitalWrite(LINE_3, LINE_ON);
    singleDisplayPilot(decine);
    delayMicroseconds(bright);
    allOff() ;
    digitalWrite(LINE_4, LINE_ON);
    singleDisplayPilot(unita);
    delayMicroseconds(bright);
    allOff();
  } else {
     Serial.println(String(ore)+":"+String(minuti)+":"+String(secondi));
  }



}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  String msg = "";

  while (1 < Wire.available()) { // loop through all but the last
    // receive byte as a character
    msg = msg + (char)Wire.read();       // print the character
  }
  msg = msg + (char)Wire.read();
  Serial.println(msg);
  if (msg.startsWith("11 ")) {
    String o =msg.substring(3,5);
    String m=msg.substring(6,8);
    String s =msg.substring(9,11);
    Serial.println(o+":"+m+":"+s);
    ore = o.toInt();
    minuti =m.toInt();
    secondi =s.toInt();
  }
  Serial.println(String(ore)+":"+String(minuti)+":"+String(secondi));

}
