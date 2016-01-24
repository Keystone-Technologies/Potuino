# Potuino
rfiduino.ino and cc3000.ino each work with their respective shield plugged in
as expected.  The goal is to merge the code into potuino.ino and have both
working together: to be able to both read RFID data and send that data via
HTTP using wireless.

Additional expectations of potuino will be documented here.

##To Do
- [x] Remap board pins
- [x] Send POST request to NodeJS server
- [ ] Dynamically update Content-Length header for multiple IDs
- [ ] Clean up wiring
- [x] Convert RFID tag number from byte array 
- [x] Make RFID scanner loop
- [x] Send RFID tag with POST request
- [ ] Respond from server with JSON

