## Downloading source
Install git if you do not have it. Download the source from gitlab:

    $ git clone git@gitlab.cs.washington.edu:syslab/diamond-src.git

## Building Diamond for x86

You can compile Diamond on Mac OSX and Linux for x86 using the
instructions below. Diamond has language binding for Java, Android,
Objective-C, Python and C++. In order to build the bindings in each
case, you'll need to set up different things.

### Set up the build environment

Diamond requires a few common tools to compile for various languages,
which you can download using your package manager of choice (e.g. apt
on Ubuntu/Debian or Macports/Homebrew on OSX):

* C++ bindings: Install CMake and either gcc or clang. Diamond builds
  as a C++ shared library by default.
* Python bindings: Install the Python Boost modules
* Java bindings: Install Maven and the JDK for compiling the Java bindings

For Ubuntu 14.04, installing the following packages should be 
sufficient to build Diamond:
   $ apt-get install build-essential cmake maven libpython-dev libboost-dev libevent-dev

### Building the source and language bindings
To compile a shared object library and both the C++ and Python
bindings:

	$ cd backend
	$ mkdir build
	$ cd build
	$ cmake ..
	$ make

To run tests for the C++ and Python bindings type in the `build` directory:

    $ make diamondtest

After you have compiled the C++ shared library, you can compile the
Java bindings for those libraries: 

	$ cd backend/src/bindings/java
    $ mvn package

To test the Java bindings, cd to the Java directory and type:

    $ mvn test


## Cross-compiling Diamond for Android

In order to cross-compile for using Diamond on Android or iOS, you
need to download the compile tools. Create a `toolchains` directory in
the `backend` directory of the Diamond source code and keep everything
that you downloaded there.

### Android
1. Install either Android Studio or the stand-alone SDK.
2. Create a directory for the Android NDK tools:

        $ mkdir backend/toolchains/android

2. Download the Android NDK from [here](http://developer.android.com/ndk/downloads/index.html).

3. The NDK unpacks itself so run:

        $ chmod u+x android-ndk-r10e-linux-x86_64.bin
        $ ./android-ndk-r10e-linux-x86_64.bin
		$ mv android-ndk-r10e-linux-x86_64 backend/toolchains/android/ndk

    Be sure to use the appropriate '.bin' filename for OSX.

4. Create a stand-alone toolchain for working with. On Linux:

		$ cd backend/toolchains/android
		$ ./ndk/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-4.9 --arch=arm --platform=android-21 --install-dir=toolchain

5. Compile the C++ Diamond library:
		
		$ cd backend
		$ mkdir build-arm
		$ cd build-arm
		$ cmake .. -DCMAKE_TOOLCHAIN_FILE=../Android.cmake
		$ make

6. Compile Java bindings for Diamond:

		$ cd backend/src/bindings/java
		$ mvn package

### Running a test Android app
The project DiamondAndroidTest in apps/test-apps is an Eclipse project containing a simple Diamond test app for Android.
The following instructions describe how to set up and run the app. Replace $DIAMOND_SRC with the path to the base Diamond
source directory.

1. Run the script build-diamond-android.sh to build the Java bindings and copy the shared libraries into the project folder:

		$ cd scripts
		$ ./build-diamond-android.sh $DIAMOND_SRC $DIAMOND_SRC/apps/test-apps/DiamondAndroidTest

2. In Eclipse, import the DiamondAndroidTest project:

	a. Go to File -> Import
	b. Choose "Existing Projects into Workspace" and click next
	c. Select $DIAMOND_SRC/apps/test-apps/DiamondAndroidTest as the root directory, make sure the box next to the project is checked, and click Finish

3. Clean the project (Project -> Clean) and close and open the project (right-click on the project in the Package Explorer -> Close Project) until it builds successfully.

4. If there are still build errors, follow the instructions in the top answer of this Stack Overflow question: http://stackoverflow.com/questions/17870881/cant-find-theme-appcompat-light-for-new-android-actionbar-support.
Let Niel know whether or not this step is necessary, and he will either remove it or describe it more formally.

5. Run the project as an Android application:

	a. Right click on the project name in the Package Explorer pane
	b. Select "Run As" -> "Android Application"
	c. Choose a device (either an emulator or a physical device) to run on
