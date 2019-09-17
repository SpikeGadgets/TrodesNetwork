# Trodes Network CMake build

## Purpose

The purpose of this library is for users of Trodes to be able to access spikes, LFP points, camera position, and other valuable data live, during their experiment, using Python or C++. The API also provides the ability for users' programs to communicate and send messages to other modules and even Trodes. Useful applications include: a live ripple detection script, graphing/plotting data, and closed loop experiments.

## Building on Linux

Before building, make sure you download and build the following static libraries: libzmq, czmq, malamute. Make sure you download and build Boost_Python as well for the Python library

To build from scratch, run the following. 

```bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/library/directory ..
make
make install
```

## Building on Windows

### Building ZeroMQ/CZMQ/Malamute

First download the three libraries, changing branches to the latest_release. Currently, libzmq can be built as static, but czmq and malamute don't work for some reason. 

Get CMake for windows, and the MSVC version used for Trodes (currently Visual Studio 14 2015, 64-bit)

```bash
cd libzmq/ # navigate to wherever libzmq is 
mkdir build && cd build
cmake -G "Visual Studio 14 2015 Win64" -DZMQ_STATIC=ON ..
cmake --build . --target install --config Release
# ... 
cd czmq/ # navigate to wherever czmq is 
mkdir build && cd build
cmake -G "Visual Studio 14 2015 Win64" -DLIBZMQ_ROOT_DIR='C:/Program Files/ZeroMQ/' ..
cmake --build . --target install --config Release
# ... 
cd malamute/ # navigate to wherever malamute is
mkdir build && cd build
cmake -G "Visual Studio 14 2015 Win64" -DLIBZMQ_ROOT_DIR='C:/Program Files/ZeroMQ/' -DCZMQ_ROOT_DIR='C:/Program Files/czmq/' ..
cmake --build . --target install --config Release
```


### Building TrodesNetwork

Now having built the three libraries, and also Boost Python, build TrodesNetwork. Change any paths in the build script if needed. 

```bash
mkdir build && cd build
cmake -G "Visual Studio 14 2015 Win64"  -DCMAKE_INSTALL_PREFIX=C:/newlibinstallsmsvc/  -DLIBZMQ_ROOT_DIR='C:/Program Files/ZeroMQ/' -DCZMQ_ROOT_DIR='C:/Program Files/czmq/' -DMALAMUTE_ROOT_DIR='C:/Program Files/malamute/' ..
cmake --build . --target install --config Release
```

This will install the C++ headers, libraries, and python packaging into the specified directory `/path/to/library/directory`. If you want to install into the default location for your system, don't include the `-DCMAKE_INSTALL_PREFIX` part of the cmake call

## Navigating source code

src/libTrodesNetwork contains the main source code for the library. Start with browsing the zeromq guide http://zguide.zeromq.org/, and then look at what CZMQ and Malamute libraries are for. Finally, start with the AbstractModuleClient class, which is what our customers/users will be using the most from this library, whether in Python or C++. 

- AbstractModuleClient: Main class for users to use or subclass from 
- CZHelp: class with static helper functions to be used across the library. 
- highfreqclasses: file that uses PUB-SUB zeromq pattern to create classes that can publish or subscribe to data streams. One pub class, and one base sub class (HFSubConsumer), and many Trodes-specific stream classes (LFP, spikes, etc)
- networkDataTypes: common data types to be transmitted across the network
- networkincludes: a poorly named file and class that contains the class MlmWrap, which is the base class of both the abstractmoduleclient class and the TrodesCentralServer class in the trodes repo
- trodesglobaltypes: a mix of more types and classes that are used all over the codebase as well as for users of this library 
- trodesmsg: a message class that wraps zmq message frames. 

## Using with C++

To use with C++, simply `#include` the headers you need, and compile against the library using `-lTrodesNetwork`. You may need to specify the header directory using `-I/path/to/library/include` and/or the library directory using `-L/path/to/library/lib`.

## Using with Python3

To use with Python (version 3.5+), navigate to the install directory and run the following:

```bash
python3 -m pip install spikegadgets_python/
```

If you are updating the library with a newer version, make sure you pass in the flag `--upgrade` at the end of that statement

To import the library, simply call `from spikegadgets import trodesnetwork`.

## Documentation

More documentation is available on the [Github pages website](https://spikegadgets.github.io/docs/networkapi/)

Created by following a [shared library CMake template](https://github.com/robotology/how-to-export-cpp-library).
