
#include <EEPROM.h>

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
    this -> command = ((command << 4) + channel) & 0x7f;
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
    this -> command = ((command << 4) + channel) & 0x7f;
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

static int manufacturers_id []= {-1, 0, 0};
static int product_id [] = {-1, 0};
static int product_model [] = {0, 0};
static int product_version [] = {0, 0, 0, 0};
static int device_id = 0x7f;

void eeprom_read () {
  int ep = 0;
  for (int ind = 0; ind < 3; ind++) {manufacturers_id [ind] = EEPROM . read (ep++);}
  product_id [0] = EEPROM . read (ep++); product_id [1] = EEPROM . read (ep++);
  product_model [0] = EEPROM . read (ep++); product_model [1] = EEPROM . read (ep++);
  for (int ind = 0; ind < 4; ind++) {product_version [ind] = EEPROM . read (ep++);}
  device_id = EEPROM . read (ep++);
  button_command * bc = button_commands;
  for (int ind = 0; ind < 12; ind++) {
    bc -> short_message = EEPROM . read (ep++) != 0;
	bc -> command = EEPROM . read (ep++);
	bc -> msb = EEPROM . read (ep++);
	bc -> lsb = EEPROM . read (ep++);
	bc -> off = EEPROM . read (ep++);
	bc++;
  }
  led_command * lc = led_commands;
  for (int ind = 0; ind < 12; ind++) {lc -> command = EEPROM . read (ep++); lc -> msb = EEPROM . read (ep++); lc++;}
  lc = knob_commands;
  for (int ind = 0; ind < 16; ind++) {lc -> command = EEPROM . read (ep++); lc -> msb = EEPROM . read (ep++); lc++;}
}

void eeprom_burn () {
  int ep = 0;
  for (int ind = 0; ind < 3; ind++) {EEPROM . update (ep++, manufacturers_id [ind]);}
  EEPROM . update (ep++, product_id [0]); EEPROM . update (ep++, product_id [1]);
  EEPROM . update (ep++, product_model [0]); EEPROM . update (ep++, product_model [1]);
  for (int ind = 0; ind < 4; ind++) {EEPROM . update (ep++, product_versin [ind]);}
  EEPROM . update (ep++, device_id);
  button_command * bc = button_commands;
  for (int ind = 0; ind < 12; ind++) {
    EEPROM . update (ep++, bc -> short_message ? 0xff : 0);
	EEPROM . update (ep++, bc -> command);
	EEPROM . update (ep++, bc -> msb);
	EEPROM . update (ep++, bc -> lsb);
	EEPROM . update (ep++, bc -> off);
	bc++;
  }
  led_command * lc = led_commands;
  for (int ind = 0; ind < 12; ind++) {EEPROM . update (ep++, lc -> command); EEPROM . update (ep++, lc -> msb); lc++;}
  lc = knob_commands;
  for (int ind = 0; ind < 16; ind++) {EEPROM . update (ep++, lc -> command); EEPROM . update (ep++, lc -> msb); lc++;}
}

int check_manufacturers_id () {
  if (midi_counter < 1) return 0;
  if (manufacturers_id [0] != 0 && manufacturers_id [0] == midi_message [0]) return 1;
  for (int ind = 0; ind < 3; ind++) {if (manufacturers_id [ind] != midi_message [ind]) return 0;}
  if (midi_counter < 3) return 0;
  return 3;
}

void process_system_exclusive () {
  if (midi_message [0] == 0x7e) {
    if (manufacturers_id [0] < 0 || product_id [0] < 0) {Serial . write (0xf0); Serial . write (0x7e); Serial . write (0x7f); Serial . write (0xf7); return;}
    if (midi_message [1] == 0x7f || midi_message [1] == device_id) {
      if (midi_message [2] != 6 || midi_message [3] != 1) return;
      Serial . write (0xf0); Serial . write (0x7e); Serial . write (device_id); Serial . write (6); Serial . write (2);
      if (manufacturers_id [0] < 1) {Serial . write (0); Serial . write (manufacturers_id [1]); Serial . write (manufacturers_id [2]);}
      else Serial . write (manufacturers_id [0]);
      Serial . write (product_id [0]); Serial . write (product_id [1]);
      Serial . write (product_model [0]); Serial . write (product_model [1]);
      for (int ind = 0; ind < 4; ind++) Serial . write (product_version [ind]);
      Serial . write (0xf7);
    }
    return;
  }
  if (midi_message [0] == 0x7d) {
    int mp = 1;
    if (midi_message [mp] == 0) {for (int ind = 0; ind < 3; ind++) {if (mp >= midi_counter) return; manufacturers_id [ind] = midi_message [mp++];}}
    else {if (mp >= midi_counter) return; manufacturers_id [0] = midi_message [mp++];}
    if (mp >= midi_counter) return; product_id [0] = midi_message [mp++];
    if (mp >= midi_counter) return; product_id [1] = midi_message [mp++];
    if (mp >= midi_counter) return; product_model [0] = midi_message [mp++];
    if (mp >= midi_counter) return; product_model [1] = midi_message [mp++];
    for (int ind = 0; ind < 4; ind++) {if (mp >= midi_counter) return; product_version [ind] = midi_message [mp++];}
  }
  int mp = check_manufacturers_id ();
  if (mp < 1) return;
  int dv = midi_message [mp++];
  if (dv != 0x7f && dv != device_id && device_id != 0x7f) return; if (mp >= midi_counter) return;
  if (midi_message [mp++] != product_id [0]) return; if (mp >= midi_counter) return;
  int m_command, m_channel, index, msb, lsb;
  switch (midi_message [mp++]) {
    case 0: if (mp >= midi_counter) return; device_id = midi_message [mp]; return;
    case 1: if (mp >= midi_counter) return;
      m_command = midi_message [mp++]; if (mp >= midi_counter) return;
      m_channel = midi_message [mp++];
      if (mp >= midi_counter) {m_command = ((m_command << 4) + m_channel) & 0x7f; for (int ind = 0; ind < 16; ind++) knob_commands [ind] . command = m_command; return;}
      index = midi_message [mp++]; if (mp >= midi_counter) return;
      knob_commands [index] . set (m_command, m_channel, midi_message [mp]);
      return;
	case 2: if (mp >= midi_counter) return;
	  m_command = midi_message [mp++]; if (mp >= midi_counter) return;
	  m_channel = midi_message [mp++];
	  if (mp >= midi_counter) {m_command = ((m_command << 4) + m_channel) & 0x7f; for (int ind = 0; ind < 12; ind++) button_commands [ind] . command = m_command; return;}
	  index = midi_message [mp++]; if (mp >= midi_counter) return;
	  msb = midi_message [mp++]; if (mp >= midi_counter) {button_commands [index] . set (m_command, m_channel, msb, 100, 0); return;}
	  lsb = midi_message [mp++]; if (mp >= midi_counter) {button_commands [index] . set (m_command, m_channel, msb, lsb, 0); return;}
	  button_commands [index] . set (m_command, m_channel, msb, lsb, midi_message [mp]);
	  return;
	case 3: if (mp >= midi_counter) return;
	  m_command = midi_message [mp++]; if (mp >= midi_counter) return;
	  m_channel = midi_message [mp++];
	  if (mp >= midi_counter) {m_command = ((m_command << 4) + m_channel) & 0x7f; for (int ind = 0; ind < 12; ind++) led_commands [ind] . command = m_command; return;}
	  index = midi_message [mp++]; if (mp >= midi_counter) return;
	  led_commands [index] . set (m_command, m_channel, midi_message [mp]);
	  return;
	case 8: factory_reset (); break;
	case 10: eeprom_read (); break;
	case 11: eeprom_burn (): break;
	default: break;
  }
}

void process_midi_command () {
  for (int ind = 0; ind < 12; ind++) {
    led_command * lc = led_commands + ind;
	if (lc -> msb == midi_message [0]) {
	  if (lc -> command == command || (lc -> command & 0xf0 == 0x80 && lc -> command & 0xef == command & 0xef)) set_led (ind, midi_message [1]);
	}
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

void factory_reset () {
  manufacturers_id [0] = -1; manufacturers_id [1] = manufacturers_id [2] = 0;
  product_id [0] = -1; product_id [1] = 0;
  product_model [0] = product_model [1] = 0;
  for (int ind = 0; ind < 4; ind++) {product_version [ind] = 0;}
  device_id = 0x7f;
  for (int ind = 0; ind < 6; ind++) {button_commands [ind] . set (0xc, 0, ind, 100, 0); led_commands [ind] . set (0xb, 0, ind);}
  for (int ind = 6; ind < 12; ind++) {button_commands [ind] . set (0x9, 0, 54 + ind, 100, 0); led_commands [ind] . set (0x9, 0, 54 + ind);}
  for (int ind = 0; ind < 16; ind++) {knob_commands [ind] . set (0xb, 0, ind);}
}

void setup () {
  factory_reset ();
  for (int ind = 0; ind < 14; ind++) {pinMode (ind, OUTPUT);}
  for (int ind = 0; ind < 16; ind++) {analogs [ind] = (a [ind] = analogRead (A0 + ind)) >> 3;}
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

