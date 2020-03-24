

mosrun
======


Table of Contents
-----------------

1. Note
1. About
1. Usage
1. Resource forks and tool binaries
1. Todo List
1. Tested and Untested Tools
1. Alternatives to mosrun
1. _DumpRex_ and _BuildRex_


Note
----

mosrun was designed as a 32 bit executable. It was ported to 64 bit in
February 2020. Only a few apps were tested and there are sure to be a
bunch of bugs and tons of inefficiencies. mosrun seems to be stable
enough though to run the typical jobs needed for developing Newton OS code.

The original 32 bit mosrun code is located at:
https://github.com/MatthiasWM/dynee5/tree/master/Alienate/mosrun


About
-----

mosrun, short for "MacOS runtime environment", is a program that runs
m68k based MPW tools on Mac OS X, Linux, and MSWindows. The main purpose is to
run the Apple Newton developer tools, such as the cross compiler and the Rex 
builder, natively and as part of a build chain. But other MPW tools will
likely run well without modification.

mosrun is a simulator for a small subset of the MacOS 7.6.6 environment, paired
with a Motorola 68020 emulator that will make the binaries run on other CPUs.
Only the first few kBytes of RAM are emulated to provide access to MacOS global
variables. The rest of the RAM is managed in an array that provides minimal
memory allocation functionality.

The emulator is a copy of Musashi's m68k. It runs perfectly for what I need
while providing a simple clean interface.

The simulator is based on Inside Macintosh I-III and what ever other
documentation I could gather from the Net. The MPW interface however is not
documented. File handling uses a Unix/BSD style interface, but is still based
on a lot of reverse engineering and guess work.

No Apple ROMs are needed.


Usage
-----

If mosrun was precompiled with the resource fork of an MPW tool, just rename 
mosrun to the name of the original tool and it will behave like that tool.

Example: `ARM6asm test.s`

As a stand-alone tool, the first argument should be the name of the MPW tool.
Note that MPW tools make intense use of the Resource Fork. If you did not 
copy the tool's resource fork, mosrun will fail to run.

Example: `mosrun MPWTools/ARM6asm test.s`

In either case, if the first argument is '---run' (triple dash option), mosrun
will try to load the MPW tool in the following argument.

Example: `mosrun ---run MPWTools/ARM6asm test.s`

Further triple dash options will follow, concerning the conversion of 
file names and data streams.

Alternate usage: If a symbolic link is created which points to mosrun, then
mosrun will use the name of the link to search for the tool binary. If for
example a link named ARMCpp is created which points to mosrun, then mosrun
will first search the directory specified in the MOSRUN_PATH variable, and
then under /usr/local/lib/mosrun/ for the MacOS binary.


Resource forks and tool binaries
--------------------------------

On Mac OS X, mosrun will open the resource fork of the NCT tools directly.
This is not an option on other platforms: In this case, mosrun will open the
tool's file name and read the code directly from there. This requires that the
tool's code is first extracted on a Mac OS X computer by copying the resource
fork, e.g. with `cat ARMCpp/rsrc > pathToTools/ARMCpp`.

As of July 28th, 2013:

1. the code basically works. There is still a lot that wants to be implemented
   and fixed, but the idea has proven to work. ARM6asm launches, opens files,
   writes text, and (almost) reads files. If no parameters are given, it 
   finishes without crashing. Yay!
1. the code itself is an utter mess though. As soon as I get mosrun to
   completely run ARM6asm, I will need to reeingineer and document the code.
1. Usage is 'mosrun ARM6asm'. If ARM6asm was correctly copied from MacOS,
   keeping the resource fork intact (that's where the code lives!), it should
   output the m68k code as it is executed, and the ARM6asm Usage text.

As of August 3rd, 2013:

1. ARM6asm seems to run with no complaints. I was able to generate object code
   from assembler files.
1. I have moved memory management into its own module, giving it some very
   useful bounds checking and allocation tracking.
1. I have added filename-separator translation. MacRoman to UTF8 translation
   will follow shortly.

As of March 2nd, 2020:

1. ARM6asm runs fine again in a 64 bit environmet
1. Memeory managment verified and running


Todo List
---------

1. write a BUILD.md file to explain how to build mosrun and more
1. verify build on Linux and MSWindows
1. make 'mosmake' part of 'mosrun'
1. write a script that generates command line executables of all NCT tools
1. create directories with real world examples


Tested and Untested Tools
-------------------------

| Tool              |    | Description / Status                                |
|-------------------|----|-----------------------------------------------------|
| AIFtoNTK          | OK | This tool converts an ARM Image Format (AIF) file into an NTK “streamed frame” file. |
|                   |    | Tool creates correct .ntkc files. **Note:** the configuration file must use Mac style newline characters! **Note:** to use the .ntkc file with MacOS NTK, set its type to 'ntkc' with *ResEdit* |
| ARM6asm           | OK | ARM AOF Macro Assembler 2.21, May 10 1994 |
|                   |    | Tool has been used successfully to create object files from assembler. |
| ARM6c             |  i | Norcroft Newton OS Newton C vsn 4.62b1, Feb 20 1995 |
|                   |    | This tool launches and needs to be tested and documented. |
| ARMCpp            |  i | Norcroft Newton OS Newton C++ vsn 0.43/C4.68, Jul 12 1996 |
|                   |    | This tool launches and needs to be tested and documented. |
| ARMLink           | OK | ARM Linker vsn 5.04, Advanced RISC Machines rel195, Apr 29 1996 |
|                   |    | Tool has been used to create AIF files. |
| DumpAIF           |  i | This is a tool for advanced users who want to see the linker output code. |
|                   |    | This tool launches and needs to be tested and documented. |
| DumpAOF           |  i | AOF Decoder vsn 3.32, decodes an ARM Object Format (AOF) file |
|                   |    | This tool launches and needs to be tested and documented. |
| ProtocolGenTool   |  i | Generate stubs for interfaces and implementatiosn |
|                   |    | At least we now know that Elvis is dead. |
| Rex               |  i | Tool to bild a ROM Extension file form various parts using a configuration file |
|                   |    | Tool creates .rex files, but those have not been tested yet. **Note:** the configuration file must use Mac style newline characters! |
| ARMCFront         |  x | Outdated ARM C++-to-C compiler |
|                   |    | No need to support this. |
| CallFinder        |  x | No documentation found. |
|                   |    | Uses graphics calls, s not emulated in the command line. |
| CoolRequest       |  x | No documentation found. |
|                   |    | Dialog boxes are not emulated in the command line. |
| LinkMenu          |  x | No documentation found. |
|                   |    | Menu items are not emulated in the command line. |
| Makemake          |  x | Probably generates a Makefile for various type of projects. |
|                   |    | This tool launches but has no description in the help page, so we don't know what it does. |
| Packer            |  x | No documentation found. |
|                   |    | This tool launches but has no description in the help page, so we don't know what it does. |
| Pram              |  x | No documentation found. |
|                   |    | This tool launches but has no description in the help page, so we don't know what it does. |
| SetMenuItem       |  x | SetMenuItem can change the title and/or style of an installed MPW user menu item. |
|                   |    | Menu items are not emulated in the command line. |

* OK = tested and working
* i = included, but not yet tested
* w = minor bugs, needs to be looked at
* e = crashing, needs to be fixed
* x = will not be included or supported

Alternatives to mosrun
----------------------

* BasiliskII, Mini VMac, and other emulators:
  * this requires to set up an entire MacOS system with MPW and tools. It is not
    (easily) possible to launch tools inside MPW from the host computer though.
* Executor:
  * https://web.archive.org/web/20090404042926/http://www.ardi.com/executor.php ,
    does what mosrun does for graphical MacOS applications. It seems to be lacking
    the MPW interface, plus I could not get it to compile.
* Classix:
  * Very similar to mosrun, but focusses on PPC. Some of the Newton tools are not
    available in PPC though, only in m68k.
* mpw:
  * https://github.com/ksherlock/mpw/blob/master/README.text came online the same
    week that I went online. Wow!
* macloader:
  * https://github.com/lordhoto/macloader tries something similar but seems to be
    inactive.
* and of course Carbon/Rosetta on OS X with PowerPC
  * http://www.apple.com
* a more recent approac is M.A.C.E.
  * https://mace.software , MACE is quite similar to Executor. Unfortunatley at the time of
    writing, MACE is not OpenSource and needs to be compiled by the author to include
    one specific application. This would be a great solution for NTK, the Newton Toolkit.


mosrun is licensed under GPL V3. No warranties, no claims, enjoy.


_DumpRex_ and _BuildRex_
------------------------

_DumpRex_ takes a NewtonOS ROM Extension file name (a file ending in _.rex_,
probably Einstein.rex) and creates a configuration file (Einstein.rex.cfg)
and another file for each block or package inside the REX. Newton Script
packages extracted from a REX should be fixed to run as a regualr package
and can also be disassembled with the appropriate tools.

_BuildRex_ takes two arguments. The first argument is the filename of the
configuration file, as created by _DumpRex_. You can add NewtonScript based
packages to the configuration file, and remove existing packages as well. The
second argument is the name of the new REX file that _BuildRex_ will generate.

Note that both tools are very experimental single-purpose projects that will
fail on more complex tasks.

Also note that modifying _Einstein.rex_ will not install or uninstall packages
automatically. For now, the only way I know is to delete the _flash_ file. It
may be possible to makea a backup first and restor it later.

 - Matthias
