## How to set up and build Diamond Android

Diamond runs in the JVM, so in order to use it, you must compile and
install a custom Android environment.

### Set up the build environment
If you are running Linux, there is pretty much nothing to do here.

If you want to compile and run Diamond on OSX, then you need to install and set up a few things:

1. Create a case-sensitive file system for Diamond to use. Run the following commands to set this up:

    $ hdiutil create -type SPARSE -fs 'Case-sensitive Journaled HFS+' -size 40g ~/android.dmg
    $ hdiutil attach ~/android.dmg.sparseimage -mountpoint android
    
2. If you ever want to unmount your file system image:
    $ hdiutil detach android

3. If you do not already have Xcode (> 4.5.2) and Command Line Tools installed, do that now.
4. If you do not have MacPorts installed, install that as well. Then run:

    $ POSIXLY_CORRECT=1 sudo port install gmake libsdl git gnupg
    
5. Increase the number of file descriptors for highly parallel compiling:

    $ ulimit -S -n 1024

For both Linux and Mac, you can do the following to improve compile times:

1. Setup a compile cache

    $ export USE_CCACHE=1

2. Once you have downloaded the code, run:

    $prebuilts/misc/linux-x86/ccache/ccache -M 50G

Or the following for Mac OSX:

    prebuilts/misc/darwin-x86/ccache/ccache -M 50G

### Downloading source
Now you can download the source from gitlab:

    $ git clone git@gitlab.cs.washington.edu:
