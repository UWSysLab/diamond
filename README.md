### Downloading source
Now you can download the source from gitlab:

    $ git clone git@gitlab.cs.washington.edu:syslab/diamond-src.git

### Set up the build environment
Diamond has language binding for Java, Android, Python and C++. In
order to build the bindings in each case, you'll need to set up
different things.

* Python
  - Install the Python Boost modules
* Java
  - Install Maven for compiling the Java bindings

### Building the source and language bindings
To compile a shared object library and both the C++ and Python
bindings, just type make in the root directory.

To run tests for the C++ bindings type:
''' $ make test

To compile the Java bindings, cd to the Java directory and type:
''' $ mvn package

To test the Java bindings, cd to the Java directory and type:
''' $ mvn test

