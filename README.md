## Downloading source
Now you can download the source from gitlab:

    $ git clone git@gitlab.cs.washington.edu:syslab/diamond-src.git

## Building Diamond

You can compile Diamond on Mac OSX and Linux for x86 or ARM using the
instructions below. Diamond has language binding for Java, Android,
Objective-C, Python and C++. In order to build the bindings in each
case, you'll need to set up different things.

### Set up the build environment

### Native builds on Mac OSX or Linux

Diamond requires a few common tools to compile for various languages,
which you can download using your package manager of choice (e.g. apt
on Ubuntu/Debian or Macports/Homebrew on OSX):

* C++ bindings: Install CMake and either gcc or clang. Diamond builds
  as a C++ shared library by default.
* Python bindings: Install the Python Boost modules
* Java bindings: Install Maven for compiling the Java bindings

### Cross-compiling

In order to cross-compile for using Diamond on Android or iOS, you
need to download the compile tools. Create a `toolchains` directory in
the `backend` directory of the Diamond source code and keep everything
that you downloaded there.

#### Android
1. Install either Android Studio or the stand-alone SDK.
2. Create a directory for the Android NDK tools:

        $ mkdir backend/toolchains/android

2. Download the Android NDK from [here](http://developer.android.com/ndk/downloads/index.html).
3. The NDK unpacks itself so run:

        $ chmod u+x android-ndk-r10e-linux-x86_64.bin
        $ ./android-ndk-r10e-linux-x86_64.bin
4. Create a stand-alone toolchain for working with

			$ mkdir -p toolchains/android
			$ mv <NDK> toolchains/android/ndk
			$ mkdir toolchains/android/toolchain
			$ <NDK>/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-4.9 --arch=arm --platform=android-21 --install-dir=toolchains/android/toolchain


To build for Android and iOS, we need to cross compile for ARM and the
Android or iOS abi.
* Android
  - Install autotools and automake, libtool


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
	
