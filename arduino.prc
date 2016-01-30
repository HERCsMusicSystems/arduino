import lunar

program arduino [driver mdi]

[[driver] [midi mdi midi_monitor "/dev/ttyACM0"]]
[[driver] [midi mdi midi_monitor "/dev/ttyACM1"]]

end := [[driver] [gtk_command] [TRY [driver]]] .

