
//#define DEBUG

static int a [16];
static int analogs [16];
static int programs [54];
static int brightness = 0;
static int current_led = -1;
static int cycles = -1;
static int msb = 0;
static int lsb = 0;
static boolean active_control_read = false;
static boolean msb_read = true;

void setup () {
  for (int ind = 0; ind < 14; ind++) {pinMode (ind, OUTPUT);}
  for (int ind = 0; ind < 16; ind++) {a [ind] = analogs [ind] = 0;}
  for (int ind = 22; ind < 54; ind++) {pinMode (ind, INPUT); programs [ind] = 0;}
  Serial . begin (9600);
}

void set_led (int led) {
  for (int ind = 0; ind < 14; ind++) {
    if (ind == led) analogWrite (ind, 100);
    else analogWrite (ind, 0);
  }
}

int to_program (int from) {
  switch (from) {
    case 22: return 0;
    case 24: return 1;
    case 28: return 2;
    case 30: return 3;
    case 32: return 4;
    case 34: return 5;
    case 52: return 6;
    case 48: return 7;
    case 50: return 8;
    case 44: return 9;
    case 42: return 10;
    case 40: return 11;
    default: break;
  }
  return from;
}

void control_processing () {
  if (0 <= msb && msb < 12) analogWrite (msb + 2, lsb);
}

void loop () {
  //analogWrite (current_led + 2, brightness);
  //brightness += 1;
  //if (brightness > 250) brightness = 0;
  for (int ind = 0; ind < 16; ind++) {
    int v = analogRead (A0 + ind);
    if (abs (v - a [ind]) > 3) {
      a [ind] = v;
      v >>= 3;
      if (v != analogs [ind]) {
        #ifdef DEBUG
        Serial . print ("control ["); Serial . print (ind); Serial . print (" "); Serial . print (v); Serial . print ("]\n");
        #else
        Serial . write (0xb0); Serial . write (ind); Serial . write (v);
        #endif
      }
      analogs [ind] = v;
    }
  }
  for (int ind = 22; ind < 54; ind += 2) {
    int v = digitalRead (ind);
    if (v != 0 && v != programs [ind]) {
      current_led = to_program (ind);
      set_led (current_led + 2);
      cycles = 100;
      Serial . write (0xc0); Serial . write (current_led);
    }
    programs [ind] = v;
  }
  while (Serial . available ()) {
    int data = 0;
    data = Serial . read ();
    if (data >> 4 == 0xb) {active_control_read = true; msb_read = true;}
    else if (data < 128) {
      if (active_control_read) {
        if (msb_read) {msb = data; msb_read = false;}
        else {lsb = data; msb_read = true; control_processing ();}
      }
    } else {active_control_read = false;}
  }
  if (cycles >= 0) cycles--;
  if (cycles == 0) set_led (-1);
  delay (100);
}

/*
static int a [16];
static int analogs [16];
static int brightness = 0;



void setup() {
  for (int ind = 0; ind < 14; ind++) {pinMode (ind, OUTPUT);}
  for (int ind = 22; ind < 54; ind++) {pinMode (ind, INPUT);}
  for (int ind = 0; ind < 16; ind++) {a [ind] = analogs [ind] = 0;}
  Serial.begin(9600);
}



void loop() {
  int v;
  for (int ind = 0; ind < 16; ind++) {
    v = analogRead (A0 + ind);
    if (abs (v - a [ind]) > 3) {
      a [ind] = v;
      v >>= 3;
      if (v != analogs [ind]) {
        #ifdef DEBUG
        Serial . print ("control ["); Serial . print (ind); Serial . print (" "); Serial . print (v); Serial . print ("]\n");
        #else
        Serial . write (0xb0); Serial . write (ind); Serial . write (v);
        #endif
      }
      analogs [ind] = v;
    }
  }
  //if (Serial . available ()) {
  //  int brightness = Serial . read ();
  //  brightness &= 0xff;
  //  analogWrite (13, brightness);
  //}
  for (int ind = 4; ind < 8; ind++) analogWrite (ind, brightness);
  brightness += 4;
  if (brightness > 250) brightness = 0;
  for (int ind = 22; ind < 54; ind++) {
    int ii = digitalRead (ind);
    if (ii != 0) {
      #ifdef DEBUG
      Serial . print ("programchange ["); Serial . print (ind); Serial . print (" "); Serial . print (ii); Serial . print ("]\n");
      #else
      Serial . write (0xb0); Serial . write (ind); Serial . write (ii);
      #endif
    }
  }
  delay (100);
  //int sensorValue = analogRead(A0);
  //float voltage = sensorValue * (5.0 / 1023.0);
  //Serial.println(voltage);
}
*/

