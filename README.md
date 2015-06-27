## How to set up and build Diamond Android

Diamond runs in the JVM, so in order to use it, you must compile and
install a custom Android environment.

### Set up the build environment
If you are running Linux, there is pretty much nothing to do here.

If you want to compile and run Diamond on OSX, then you need to install and set up a few things

1. Create a case-sensitive file system for Diamond to use. Run the following commands to set this up:

    $ hdiutil create -type SPARSE -fs 'Case-sensitive Journaled HFS+' -size 40g ~/android.dmg
    $ hdiutil attach ~/android.dmg.sparseimage -mountpoint android
    
2. If you ever want to unmount your file system image:
    $ hdiutil detach android

3. If you do not already have Xcode (> 4.5.2) and Command Line Tools installed, do that now.
4. If you do not have MacPorts installed, install that as well. Then run:

    $ POSIXLY_CORRECT=1 sudo port install gmake libsdl git gnupg