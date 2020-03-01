

mosrun
======


Note
----

mosrun was designed as a 32 bit executable. This is an effort to port mosrun
to 64 bit, as required by macOS and soon all new operating systems.

Mamory allocation and perticularly freeing memory is not yet implemented
all the way.

At this point, mosrun compiles and links and runs the embedded code. The
FileIO functions for duplicating file descriptors and testing if a file
descriptor points a terminal need to be implemented.

Everything must be throughly tested still.

If you want the original mosrun code, go to
https://github.com/MatthiasWM/dynee5/tree/master/Alienate/mosrun

This mesaage will be remove as soon as morun executes in 64 bit systems.


About
-----

mosrun, short for "MacOS runtime environment", is a program that runs
m68k based MPW tools on Mac OS X, Linux, and MSWindows. The main purpose is to
run the Apple Newton developer tools, such as the cross compiler and the Rex 
builder, natively and as part of a build chain. But other MPW tools will
likely run well without modification.

mosrun is a simulator for a small subste of the MacOS 7.6.6 environment, paired
with a Motorola 68020 emulator that will make the binaries run on other CPUs.
Only the first few kBytes of RAM are emulated to provide access to MacOS global
variables. The rest of the RAM is managed in a array that provides minimal
memory allocation functionality.

The emulator is a copy of Musashi's m68k. It runs perfectly for what I need,
providing a simple clean interface.

The simulator part is based on Inside Macintosh I-III and what ever other 
documentation I could gather from the Net. The MPW interface however is not
documented. File handling uses a Unix/BSD style interface, but is still based
on a lot of reverse engineering and guess work.

No Apple ROMs are needed.


Usage
-----

If mosrun was precompiled with the resource fork of an MPW tool, just rename 
mosrun to the neame of the original tool and it will behave like that tool.

Example: > ARM6asm test.s

As a stand-alone tool, the first argument should be the name of the MPW tool.
Note that MPW tools make intense use of the Resource Fork. If you did not 
copy the tool's resource fork, mosrun will fail to run.

Example: > mosrun MPWTools/ARM6asm test.s

In either case, if the first argument is '---run' (triple dash option), mosrun
will try to load the MPW tool in the following argument.

Example: > mosrun ---run MPWTools/ARM6asm test.s

Further triple dash options will follow, concerning the conversion of 
file names and data streams.

Alternate usage: If a symbolic link is created which points to mosrun, then
mosrun will use the name of the link to search for the tool binary. If for
example a link named ARMCpp is created which points to mosrun, then mosrun
will search first the directory specified in the MOSRUN_PATH variable, and
then under /usr/local/lib/mosrun/ for the binary code.


Resource forks and tool binaries
--------------------------------

On Mac OS X, mosrun will open the resource fork of the NCT tools directly.
This is not an option on other platforms: In this case, mosrun will open the
tool's file name and read the code directly from there. This requires that the
tool's code is first extracted from a Mac OS X computer by copying the resource
fork, e.g. with "cat ARMCpp/rsrc > <path to new tool>/ARMCpp".

As of July 28th, 2013:

1. the code basically works. There is still a lot that wants to be implemented
   and fixed, but the idea has proven to work. ARM6asm launches, opens files,
   writes text, and (almost) reads files. If no parameters are given, it 
   finishes without crashing. Yay!
   
2. the code itself is an utter mess though. As soon as I get mosrun to 
   completely run ARM6asm, I will need to reeingineer and document the code.
   
3. Usage is 'mosrun ARM6asm'. If ARM6asm was correctly copied from MacOS,
   keeping the resource fork intact (that's where the code lives!), it should
   output the m68k code as it is executed, and the ARM6asm Usage text.

As of August 3rd, 2013:

1. ARM6asm seems to run with no complaints. I was able to generate object code
   from assembler files.

2. I have moved memory management into its own module, giving it some very 
   useful bounds checking and allocation tracking.
   
3. I have added filename-separator translation. MacRoman to UTF8 translation
   will follow shortly.
   

Alternatives to mosrun
----------------------

BasiliskII, Mini VMac, and other emulators: 
  this requires to set up an entire MacOS system with MPW and tools. It is not
  (easily) possible to launch tools inside MPW from the host computer though.
Executor:
  Does what mosrun does for graphical MacOS applications. It seems to be lacking 
  the MPW interface, plus I could not get it to compile.
Classix:
  Very similar to mosrun, but focusses on PPC. Some of the Newton tools are not
  available in PPC though, only in m68k.
mpw:
  https://github.com/ksherlock/mpw/blob/master/README.text came online the same 
  week that I went online. Wow!
macloader:
  https://github.com/lordhoto/macloader tries something similar but seems to be
  inactive.
and of course Carbon/Rosetta on OS X with PowerPC
  http://www.apple.com


mosrun is licensed under GPL V3. No warranties, no claims, enjoy.

