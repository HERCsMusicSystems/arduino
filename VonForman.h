
class Program {
public:
	struct {
		double H, S, V, R, G, B; double RGB;
		void ground (void) {S = V = 1.0; H = R = G = B = RGB = 0.0;};
	} Colour;
	struct {
		double Attack, Decay, Sustain, Release, Width;
		void ground (void) {Attack = Release = 0.0; Decay = Sustain = 1.0; Width = 2.0;}
	} Width;
	struct {
		double Attack, Decay, Sustain, Release;
		void ground (void) {Attack = Release = 0.0; Decay = Sustain = 1.0;};
	} Intensity;
	double Mono;
	void VoiceInit (void) {Colour . ground (); Width . ground (); Intensity . ground (); Mono = 0.0;};
};

class Key {
public:
	Program * program;
	void KeyOn (int velocity);
	void KeyOff (void);
	void Move (double time);
	void Ground (Program * program) {this -> program = program;};
};

class Channel {
public:
	Key Keys [128], MonoKey;
	int KeyStack [128], KeyStackPointer;
	Program program;
	void RemoveKey (int key) {
		int ind = 0;
		while (ind < KeyStackPointer) {
			if (KeyStack [ind] == key) {for (int sub = ind; sub < KeyStackPointer - 2; sub ++) KeyStack [sub] = KeyStack [sub + 1]; KeyStackPointer --;}
			ind ++;
		}
	};
	void KeyOn (int key, int velocity) {
		RemoveKey (key);
		if (KeyStackPointer >= 128) KeyStackPointer = 127;
		KeyStack [KeyStackPointer ++] = key;
		if (KeyStackPointer < 2) MonoKey . KeyOn (velocity);
		Keys [key] . KeyOn (velocity);
	};
	void KeyOff (int key) {RemoveKey (key); if (KeyStackPointer < 1) MonoKey . KeyOff (); Keys [key] . KeyOff ();};
	void KeyOff (void) {for (int ind = 0; ind < 128; ind ++) Keys [ind] . KeyOff (); MonoKey . KeyOff (); KeyStackPointer = 0;};
	void Ground (void) {for (int ind = 0; ind < 128; ind ++) Keys [ind] . Ground (& program); KeyStackPointer = 0;};
};

