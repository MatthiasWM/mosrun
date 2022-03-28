
Building mosrun
============


Table of Contents
-----------------

1. Build variants
1. CMake
1. MacOS
    1. Xcode (verified with Xcode 11.3)
    1. Makefiles  (verified with Xcode 11.3)
1. Linux
1. Makefiles (verified on Ubuntu 18.04.3 LTS)
1. MSWindows
    1. VisualC (untested)


Build Variants
--------------

Notes on building _DumpRex_ and _BuildRex_: these target have been excluded
from the MSWindows build because they have not been ported yet. It's not a
big deal, but I just don't have the time right now.


CMake
-----

mosrun uses _CMake_ for building the executable on all platforms.


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

```bash
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

```bash
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

```bash
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

I have tested this setup in Windows 10. You will need a bunch of software:

* a _git_ client to download the source code; I really like _TortoiseGIT_ . Github also offers
  the entire source tree as an archive
* you will need _CMake_
* and you need a C++ compiler; I have added instructions for _VisualC 2017_ which is
  available from Microsoft in a free version, but takes quite a while to install. Cygwin in
  MinGW are more Unix-like alternatives

#### Downloading the source code

After installing _TortoiseGIT_, you will find some new menu items in your _File Explorer_
menus. Navigate into your development directory. As an example, I created
`C:/Users/matt/dev/` on my machine.

Right-click and choose _Git clone..._ from the context menu to copy the source code from
_github_ to the developer directory.

TortoiseGIT will pop up a window with a form that we need to fill out. The important fields
are _URL:_, set that to `https://github.com/MatthiasWM/mosrun.git`, and _Directory:_,
which should be prefilled with your dev dir, in my case `C:\Users\matt\dev\mosrun`.

Click _OK_, let it download the files, then click _Close_.


MSWindows: VisualC
-----------------

#### Creating the VisualC Solution using CMake

Launch _CMake_ and fill out the form:
* _Where is the source code:_ `C:/Users/matt/dev/mosrun`
* _Where to build the binaries:_ `C:/Users/matt/dev/mosrun/Build/VisualC/`

Then click _Configure_. If _Cmake_ asks if it should create a directory, click _yes_.

_CMake_ will ask you to _Specify the Generator for this project_. Choose
`Visual Studio 15 2017 Win64` from the pulldown menu. Click _Finish_.

Back in the _CMake_ window, click _Generate_ and _CMake_ will create all files need
by _VisualC_.

Now you can either launch _VisualC 2017_ and navigate to
`C:/Users/matt/dev/mosrun/Build/VisualC/mosrun.sln`, or simply double-click
that file in the _Explorer_, and Windows will launch _VisualC_ for you.

In _VisualC_, the left toolbox lists all target within our Solution. Right-click on _mosrun_ and
select _Set as Startup Project_ from the context menu. Then press `F7` to compile the tool,
or `F5` to run it.

The final executable will be located at your developer directory version of
`C:/Users/matt/dev/mosrun/Build/VisualC/Debug/mosrun.exe` .
