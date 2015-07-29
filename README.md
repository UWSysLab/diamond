## Downloading source
Now you can download the source from gitlab:

    $ git clone git@gitlab.cs.washington.edu:syslab/diamond-src.git

## Set up the build environment Diamond has language binding for Java,
Android, Objective-C, Python and C++. In order to build the bindings
in each case, you'll need to set up different things.

* C++
  - Install CMake and either gcc or clang. Diamond builds as a C++
    shared library by default.
* Python
  - Install the Python Boost modules
* Java
  - Install Maven for compiling the Java bindings

To build for Android and iOS, we need to cross compile for ARM and the
Android or iOS abi.
* Android
  - Install either Android Studio or the stand-alone SDK
	1. To install the SDK, download and unpack it for your platform
	2. Run:

            $ tools/android update sdk --no-ui

  - Install the NDK
	1. Download the NDK.
	2. Unpack it by running:

			$ chmod u+x <NDK>.bin
			$ mkdir -p toolchains/android
			$ mv <NDK> toolchains/android

## Building the source and language bindings
To compile a shared object library and both the C++ and Python
bindings:

	$ mkdir build
	$ cd build
	$ cmake ..
	$ make

To run tests for the C++ and Python bindings type:

    $ make test

To compile the Java bindings, cd to the Java directory and type:

    $ mvn package

To test the Java bindings, cd to the Java directory and type:

    $ mvn test

To compile for Android:

	$ mkdir android-build
	$ cd android-build
	$ cmake .. -DCMAKE_TOOLCHAIN_FILE=../Android.cmake
	$ make
	
