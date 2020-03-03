
Building mosrun
============

1. Build variants
1. CMake
1. MacOS
1. Xcode (verified with Xcode 11.3)
    1. Makefiles  (verified with Xcode 11.3)
1. Linux
    1. Makefiles (untested)
1. MSWindows
    1. VisualC (untested)


Build Variants
--------------

How to compile with integrated resources.


CMake
-----

mosrun uses CMake for building the exectable on all platforms.


MacOS
-----
    
Starting with MacOS Catalina (10.15), all executables are compiled in 64 bit mode.  

To build _mosrun_ you need to install [_CMake_](https://cmake.org/download/) and make 
it available as a command line tool by running 
`sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install` . 
I verified building with _CMake 3.15.4_ .

Make sure you have _Xcode_ and its command line tools installed. _Xcode_ is available
for free in the Apple App Store. I have tested building with _Xcode 11.3.1_ .


MacOS: Xcode
-----------

Run the following commands from your Unix shell inside your developer directory.
This will create command line executables in `mosrun/Build/Xcode/Debug/` .

Of course you can use the _Xcode_ graphical interface as well. After the call to _CMake_,
enter the command  `open mosrun.xcodeproj` and press _Apple-B_ to build the
program.

```
git clone https://github.com/MatthiasWM/mosrun.git mosrun
cd mosrun
mkdir Build && cd Build
mkdir Xcode && cd Xcode
cmake -G Xcode ../..
xcodebuild
./Debug/mosrun
```


MacOS: Makefiles
----------------

Run the following commands from your Unix shell inside your developer directory.
This will create command line executables in `mosrun/Build/Makefiles/` .

```
git clone https://github.com/MatthiasWM/mosrun.git mosrun
cd mosrun
mkdir Build && cd Build
mkdir Makefiles && cd Makefiles
cmake -G "Unix Makefiles" ../..
make
./mosrun
```


Linux
-----

Linux comes in many variations in various destribution. Different distros use differnt
tools to install programs and other files.

You will need:
* a C++ compiler, for example _g++_
* the Makefile sytem, for example _gmake_
* the build file builder _CMake_


Linux: Makefiles
----------------

Run the following commands from your Unix shell inside your developer directory.
This will create command line executables in `mosrun/Build/Makefiles/` .

```
git clone https://github.com/MatthiasWM/mosrun.git mosrun
cd mosrun
mkdir Build && cd Build
mkdir Makefiles && cd Makefiles
cmake -G "Unix Makefiles" ../..
make
./mosrun
```


MSWindows
---------

Untested


MSWindows: VisualC
-----------------

Untested


