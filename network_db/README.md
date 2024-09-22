# Overview
This program uses multiplexing to handle synchronous I/O operations such as loading the contents of a file into memory, manipulating said memory based on network requests received from a remote client while also maintaining a state.

It uses POSIX api such as socket and poll.
# Build
Run `make` in the root directory. The binaries should be build under the `bin` directory.