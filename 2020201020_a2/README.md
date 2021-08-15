# OS Assignment 2

2 cpp files are used: `peer.cpp` and `tracker.cpp`

`Tracker's IP Address:` 127.0.0.1
`Tracker's Port No:` 8880

### Compiling Peer and Tracker side programs:

###### For `tracker.cpp` program:

g++ tracker.cpp -lpthread -lssl -lcrypto -o tracker

###### For `peer.cpp` program:

g++ peer.cpp -lpthread -lssl -lcrypto -o peer

### Executing Peer and Tracker side programs:

###### For `tracker.cpp` program:

./tracker

###### For `peer.cpp` program:

./peer 127.0.0.1 7779 127.0.0.1 8880 (For example)

`NOTE:` The `first` and `second` arguments after `./peer` denote sender peer's IP Address and Peer's Port number.

The `third` and `fourth` arguments after `./peer` denote receiver peer's/tracker's IP Address and Peer's/Tracker's Port number.

















