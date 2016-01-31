
void set_led (int led, int intensity) {
  if (led < 0) return;
  if (led < 6) {analogWrite (7 - led, intensity); return;}
  if (led < 12) analogWrite (19 - led, intensity);
}

struct button_command {
  boolean short_message;
  int command;
  int msb;
  int lsb;
  int off;
  void set (int command, int channel, int msb, int lsb, int off) {
    this -> command = (command << 4) + channel;
    this -> msb = msb;
    this -> lsb = lsb;
    this -> off = off;
    if (command == 0xc || command == 0xd) short_message = true;
    else short_message = false;
  }
};

struct led_command {
  int command;
  int msb;
  void set (int command, int channel, int msb) {
    this -> command = (command << 4) + channel;
    this -> msb = msb;
  }
};

static button_command button_commands [12];
static led_command led_commands [12];
static led_command knob_commands [16];

static int a [16];
static int analogs [16];
static int programs [54];

static int command = -1;
static int cmd = -1;
static int channel = 0;
static int midi_message [16];
static int midi_counter = 0;

void process_system_exclusive () {
}

void process_midi_command () {
  for (int ind = 0; ind < 12; ind++) {
    led_command * lc = led_commands + ind;
    if (lc -> command == command && lc -> msb == midi_message [0]) set_led (ind, midi_message [1]);
  }
}

void process_midi (int v) {
  if (v == 0xf7) {process_system_exclusive (); return;}
  if (v == 0xf0) {command = 0xf0; cmd = 0xf0; channel = 0; midi_counter = 0; return;}
  if (v > 0xf0) return;
  if (v >= 128) {command = v; cmd = v >> 4; channel = v & 0xf; midi_counter = 0; return;}
  switch (cmd) {
    case 0xf0: if (midi_counter < 16) midi_message [midi_counter++] = v; return;
    case 0x8: case 0x9: case 0xb: midi_message [midi_counter++] = v; if (midi_counter >= 2) {process_midi_command (); midi_counter = 0;} return;
    default: return;
  }
}

void setup () {
  for (int ind = 0; ind < 6; ind++) {button_commands [ind] . set (0xc, 0, ind, 100, 0); led_commands [ind] . set (0xb, 0, ind);}
  for (int ind = 6; ind < 12; ind++) {button_commands [ind] . set (0x9, 0, 54 + ind, 100, 0); led_commands [ind] . set (0x9, 0, 54 + ind);}
  for (int ind = 0; ind < 14; ind++) {pinMode (ind, OUTPUT);}
  for (int ind = 0; ind < 16; ind++) {a [ind] = analogs [ind] = 0; knob_commands [ind] . set (0xb, 0, ind);}
  for (int ind = 22; ind < 54; ind++) {pinMode (ind, INPUT); programs [ind] = 0;}
  Serial . begin (9600);
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

void button_processing (int button, int value) {
  if (button < 0 || button >= 12) return;
  button_command * bc = button_commands + button;
  if (bc -> short_message) {
    if (value == 0) return;
    Serial . write (bc -> command); Serial . write (bc -> msb);
  } else {
    Serial . write (bc -> command); Serial . write (bc -> msb);
    Serial . write (value == 0 ? bc -> off : bc -> lsb);
  }
}

void knob_processing (int knob, int value) {
  if (knob < 0 || knob >= 16) return;
  led_command * kc = knob_commands + knob;
  Serial . write (kc -> command); Serial . write (kc -> msb); Serial . write (value);
}

void loop () {
  for (int ind = 0; ind < 16; ind++) {
    int v = analogRead (A0 + ind);
    if (abs (v - a [ind]) > 3) {
      a [ind] = v;
      v >>= 3;
      if (v != analogs [ind]) knob_processing (ind, v);
      analogs [ind] = v;
    }
  }
  for (int ind = 22; ind < 54; ind += 2) {
    int v = digitalRead (ind);
    if (v != programs [ind]) button_processing (to_program (ind), v);
    programs [ind] = v;
  }
  while (Serial . available ()) {process_midi (Serial . read ());}
  delay (100);
}

