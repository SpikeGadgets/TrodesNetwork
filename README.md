# Trodes Network CMake build

## Building

To build from scratch, run the following

```bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/library/directory ..
make
make install
```

This will install the C++ headers, libraries, and python packaging into the specified directory `/path/to/library/directory`. If you want to install into the default location for your system, don't include the `-DCMAKE_INSTALL_PREFIX` part of the cmake call

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
