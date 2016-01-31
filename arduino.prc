import lunar

program arduino [driver mdi led]

[[driver] [midi mdi midi_monitor "/dev/ttyACM0"]]
[[driver] [midi mdi midi_monitor "/dev/ttyACM1"]]

[[led *led *intensity] [mdi control 0 *led *intensity]]
[[led *led] [mdi control 0 *led 0]]

end := [[driver] [gtk_command] [TRY [driver]]] .

