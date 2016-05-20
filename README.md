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

    $ apt-get install build-essential cmake maven libpython-dev libboost-dev libboost-python-dev libssl-dev libprotobuf-dev protobuf-compiler libevent-dev

### Building the source and language bindings
To compile a shared object library and both the C++ and Python
bindings:

	$ cd platform
	$ mkdir build
	$ cd build
	$ cmake ..
	$ make

To run tests for the C++ and Python bindings type in the `build` directory:

    $ make test

After you have compiled the C++ shared library, you can compile the
Java bindings for those libraries: 

	$ cd platform/bindings/java
    $ mvn package

To test the Java bindings, cd to the Java directory and type:

    $ mvn package -DskipTests


## Cross-compiling Diamond for Android

In order to cross-compile for using Diamond on Android or iOS, you
need to download the compile tools. Create a `toolchains` directory in
the `platform` directory of the Diamond source code and keep everything
that you downloaded there.

### Android
1. Install either Android Studio or the stand-alone SDK.

2. Download the Android NDK from [here](http://developer.android.com/ndk/downloads/index.html).

3. Unpack the NDK:

        $ unzip android-ndk-r11-linux-x86_64.zip
		$ mv android-ndk-r11-linux-x86_64.tar.bz2 platform/toolchains/android/ndk

    Be sure to use the appropriate '.zip' filename for OSX.

4. Create a stand-alone toolchain for working with. On Linux:

		$ cd platform/toolchains/android
		$ ./ndk/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-4.9 --arch=arm --platform=android-21 --install-dir=toolchain

5. Add the toolchain binaries to your path. If $DIAMOND_SRC is the path to the base Diamond source directory,
add the following line to your bashrc file:

		$ export PATH=$DIAMOND_SRC/platform/toolchains/android/symlinks:$PATH

6. Compile libevent for Android:

		$ cd external/libevent
		$ ./autogen.sh
		$ ./configure --target=arm-linux-androideabi --host=arm-linux-androideabi --disable-openssl LDFLAGS=-lgnustl_shared
		$ make

7. Compile libprotobuf for Android:

		$ cd external/protobuf-2.5.0
		$ ./autogen.sh
		$ ./configure --target=arm-linux-androideabi --host=arm-linux-androideabi LDFLAGS=-lgnustl_shared
		$ make

8. Compile the C++ Diamond library:
		
		$ cd platform
		$ mkdir build-arm
		$ cd build-arm
		$ cmake .. -DCMAKE_TOOLCHAIN_FILE=../Android.cmake
		$ make

9. Compile Java bindings for Diamond:

		$ cd platform/bindings/java
		$ mvn package -DskipTests

### Running an Android app in Android Studio
The project DiamondAndroidTest in `apps/test-apps` is an Android Studio project
containing a simple Diamond test app for Android.  The following instructions
describe how to set up and run the app. Replace `$DIAMOND_SRC` with the path to
the base Diamond source directory. These instructions have been tested on
Ubuntu 14.04 using an Android SDK with Android 6.0 (API level 23) installed.

1. Run the script `copy-dependencies-android-studio.sh` to copy all required shared libraries into the project folder:

        $ cd scripts/build-scripts
        $ ./build-diamond-android-eclipse.sh $DIAMOND_SRC $DIAMOND_SRC/apps/test-apps/DiamondAndroidTest

2. In Android Studio, import the DiamondAndroidTest project:

    1. Go to File -> New -> Import Project...
    2. Select `$DIAMOND_SRC/apps/test-apps/DiamondAndroidTest` as the root directory and click Finish.

3. Add the Diamond libraries to the project:

    1. Click the Project button on the left-hand border of the window to view the Project pane.
    2. Select the Project view in the drop-down menu at the top of the pane.
    3. Navigate to the `app/src/main/libs` directory.
    4. Right-click on each library in the folder and select "Add As Library..."

4. Run the project as an Android application:

    1. Either click the green Run button in the toolbar or select Run -> Run 'app' on the menu.

### Running an Android app in Eclipse
Most of the Android apps in this repository have been moved over to Android
Studio at this point, but a few older apps may still be Eclipse projects (see
the README in the `apps` directory for details). Here we provide our old
instructions on getting an Android app running with Eclipse.

Google no longer supports developing Android apps in Eclipse. The link to the
Eclipse Android Development Tools plugin is no longer on the official Android
website, although it can be found in a mirror of the old version
(https://stuff.mit.edu/afs/sipb/project/android/docs/sdk/installing/installing-adt.html).
Also, the process of compiling and running Android apps seems to be
significantly less flaky in Android Studio than in Eclipse. For these reasons,
we recommend using Android Studio to develop Diamond Android apps.

When reading these instructions, replace
`DiamondAndroidTest` with the name of your Eclipse project, and replace
`$DIAMOND_SRC` with the path to the base Diamond source directory and
`$ANDROID_SDK` with the path to the Android SDK folder. These instructions have
been tested on Mac and Linux using an Android SDK with Android 5.1.1 (API level
22) installed.

1. Run the script `copy-dependencies-android-eclipse.sh` to copy all required shared libraries into the project folder:

        $ cd scripts/build-scripts
        $ ./copy-dependencies-android-eclipse.sh $DIAMOND_SRC $DIAMOND_SRC/apps/test-apps/DiamondAndroidTest

2. In Eclipse, import the DiamondAndroidTest project:

    1. Go to File -> Import -> General -> Existing Projects into Workspace.
    2. Select `$DIAMOND_SRC/apps/test-apps/DiamondAndroidTest` as the root directory and click Finish.

3. Add the Android support v7 appcompat library to the project:

    1. Go to File -> Import -> General -> Existing Projects into Workspace.
    2. Select `$ANDROID_SDK/extras/android/support/v7/appcompat` as the root directory and click Finish.
    3. Right click on android-support-v7-appcompat in the Package Explorer, then go to Android, and select Android 5.1.1 as the build target.
    4. Go to Project -> Properties -> Android. Under Library, select Add, then select appcompat\_v7.

4. Go to Window (or Eclipse on Mac) -> Preferences -> Android -> Build, and uncheck the option "Force error when external jars contain native libraries."

5. Refresh, clean, and close and open the project until it builds successfully:

    1. To refresh: right-click on the project in the Package Explorer -> Refresh
    2. To clean: select Project -> Clean
    3. To close and open: right-click on the project -> Close Project (or Open Project)

6. Run the project as an Android application:

    1. Right click on the project name in the Package Explorer pane.
    2. Select "Run As" -> "Android Application."
    3. Choose a device (either an emulator or a physical device) to run on.
