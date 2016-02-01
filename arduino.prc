import lunar

program arduino [driver mdi led]

[[driver] [midi mdi midi_monitor "/dev/ttyACM0"]]
[[driver] [midi mdi midi_monitor "/dev/ttyACM1"]]

[[led : *x] [mdi sysex : *x]]

end := [[driver] [gtk_command] [TRY [driver]]] .

