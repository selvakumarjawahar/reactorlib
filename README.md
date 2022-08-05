# reactorlib

This project was created for my [cppcon India 2022](https://www.cppindia.co.in/) talk. This project implements a simple reactor using epoll. The code is documented and easy to understand. Check the unit tests for sample usage. Watch the talk for more details on the reactor pattern.

## Build

As this project is using epoll, this library will work only on linux machines. The easiest way to build is to use the utility.sh. Run <br>
```bash
./utiltiy.sh -h
```
<br>
for help. This script uses docker for building the library and run tests. If you do not want to use docker for build, you can build using cmake. <br>

```bash
cd reactorlib
mkdir build
cd build
cmake ..
make
```

<br>
The only requirement for the library is C++17 compiler. The unit tests uses catch2.x, address sanitizer (libasan), ub sanitizer(ubsan). So you will need to install all these dependencies as well for running unit tests.
