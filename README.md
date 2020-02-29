

================================================================================
    mosrun
================================================================================
http://code.google.com/p/dynee5/source/browse/#svn%2Ftrunk%2FAlienate

mosrun, short for "MacOS runtime environment", is a program that will run 
arbitray MPW tools on Mac OS X, Linux, and MSWindows. The main purpose is to 
run the Apple Newton developer tools, such as the cross compiler and the Rex 
builder, natively and as part of a build chain. But other MPW tools will
likely run well without modification.

mosrun is a simulator for a MacOS 7.6.6 environment, paired with a Motorola
68020 emulator that will make the binaries run on other CPUs. Only the
first few kBytes of RAM are emulated to provide better access to MacOS global 
variables. The rest of the RAM is used directly, requiring the MPW Tools to
be linked in 'far' mode (32 bit vs. 24 bit). The host appliction needs to run 
in 32 bit mode as well (not in 64 bit mode!).

The emulator is a copy of Musashi's m68k. It runs perfectly for what I need,
providing a simple clean interface.

The simulator part is based on Inside Macintosh I-III and what ever other 
documentation I could gather from the 'net. It is currently still quite
messy as this project is just comming out of proof-of-concept stage.

The MPW interface is not documented. File handling uses a Unix/BSD style 
interface, but is still based on a lot of reverse engineering and guess work.

No Apple ROMs are needed.


USAGE:

If mosrun was precompiled with the resource fork of an MPW tool, just rename 
mosrun to the neame of the original tool and it will behave like that tool.

Example: > ARM6asm test.s

As a stand-alone tool, the first argument should be the name of the MPW tool.
Note that MPW tools make intense use of the Resource Fork. If you did not 
copy the tool's resource fork, mosrun will fail to run.

Alternate usage: If a symbolic link is created which points to mosrun, then
mosrun will use the name of the link to search for the tool binary. If for
example a link named ARMCpp is created which points to mosrun, then mosrun
will search first the directory specified in the MOSRUN_PATH variable, and
then under /usr/local/lib/mosrun/ for the binary code.

Example: > mosrun MPWTools/ARM6asm test.s

In either case, if the first argument is '---run' (triple dash option), mosrun
will try to load the MPW tool in the following argument.

Example: > mosrun ---run MPWTools/ARM6asm test.s

Further triple dash options will follow, concerning the conversion of 
file names and data streams.

RESOURCE FORKS AND TOOL BINARIES:

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
   

Alternatives:

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


mosrun is licensed under GPL V3. No warranties, no claim, enjoy.



================================================================================
EVERYTHING BELOW IS JUST AN UNSORTED COLLECTION OF SCARPS
================================================================================

How to send Keystrokes to BasiliskII and MPW:

/*
 
 tell application "BasiliskII"
 activate
 -- macrun
 
 tell application "System Events"
 keystroke "f" using {shift down}
 keystroke "i"
 keystroke "l"
 keystroke "e"
 keystroke "s"
 key code 76 -- 36 for return
 end tell
 end tell
 
 */


//------------------------------------------------------------------------------
// memory management
//
// Top:
//   Application Partition
//     "BufPtr"
//     A5 World
//       Jump Table
//       Application Parameters (28 bytes)
//       Ptr To QuickTime Globals (4 bytes)
//       "CurrentA5"
//       Application Global Variables
//       QuickDraw global variables
//     "CurrStackBase"
//     Stack (32k)
//     (space) "ApplLimit"
//     Heap
//       Relocatable Blocks
//       Non-Relocatable Blocks
//       Master Pointers
//       Resource Map
//     "AppleZone"
//   System Partition
//     (space)
//     System Heap
//     "0xb00"
//     "0x800"
//     Trap Table
//     "0x400"
//     System global Variables
//     "0"
// Bottom:

// %A5Init
// NewHandle(size), NewHandleClear(size), DisposeHandle(addr)
// NewPtr(size), NewPtrClear(size), DisposePtr(addr)



/* Resource Manager Error codes:
 badExtResource	-185
  The extended resource has a bad format.
 CantDecompress	-186
  Can’t decompress a compressed resource.
 resourceInMemory	-188
  The resource is already in memory.
 writingPastEnd	-189
  Writing past the end of file.
 inputOutOfBounds	-190
  The offset or count is out of bounds.
 resNotFound	-192
  The resource was not found.
 resFNotFound	-193
  The resource file was not found.
 addResFailed	-194
  The AddResource function failed.
 rmvResFailed	-196
  The RemoveResource function failed.
 resAttrErr	-198
  The attribute is inconsistent with the operation.
 mapReadErr	-199
  The map is inconsistent with the operation.
 
 and noErr (0), dirFulErr (-33), dskFulErr (-34), nsvErr (-35), ioErr (-36), bdNamErr (-37), eofErr (-39), tmfoErr (-42), fnfErr (-43), wPrErr (-44), fLckdErr (-45), vLckdErr (-46), dupFNErr (-48), opWrErr (-49), permErr (-54), extFSErr (-58), memFullErr (-108), dirNFErr (-120)
 
 
 Memory Manager errors are:
 menuPrgErr
 84
 A menu was purged.
 negZcbFreeErr
 33
 A heap has been corrupted.
 memROZErr
 -99
 Operation on a read-only zone. This result code is not relevant in Mac OS X.
 memFullErr
 -108
 Not enough memory in heap.
 nilHandleErr
 -109
 Handle argument is NULL.
 memAdrErr
 -110
 Address is odd or out of range.
 memWZErr
 -111
 Attempt to operate on a free block.
 memPurErr
 -112
 Attempt to purge a locked or unpurgeable block.
 memAZErr
 -113
 Address in zone check failed.
 memPCErr
 -114
 Pointer check failed.
 memBCErr
 -115
 Block check failed.
 memSCErr
 -116
 Size check failed.
 memLockedErr
 -117
 Block is locked.
 
 
 
 https://developer.apple.com/library/mac/#documentation/Carbon/reference/Resource_Manager/DeprecationAppendix/AppendixADeprecatedAPI.html
*/


// TODO: 0x002A0382: dc.w    $a820; opcode 1010 (_Get1NamedResource)
//  param ResType, Str255 name, returns handle to resource
// 0001DA0E: A994            '..'            _CurResFile                     ; A994
//  returns ResFileRefNum, the file number of the executable's resource
//  used in '__CplusInit', 'dtors__Fv' and one more
// 0001DA14: A9A4            '..'            _HomeResFile                     ; A9A4
//  handle to a resource, returns the FileID of the res file that holds this resource
// 000004BA: A02E            '..'            _BlockMove                     ; A02E
//  memmove src, dst, size
// 00000788: A020            '. '            _SetPtrSize                     ; A020
//  ptr, newsize, no return (See MemError)! Attempt to resize a previously allocated, non-movable block - may very well fail!
//  used in '__growFileTable'
//*00000790: A11E            '..'            _NewPtr                      ; A11E
//  size, returns a ptr or null
// 000007B0: A01F            '..'            _DisposePtr                     ; A01F
//  ptr to memory, free()
// 00000016: A9EE            '..'            _DECSTR68K                     ; A9EE
//  converts floating point numbers into strings and back
//  CStr2Dec               4    NumToString            0    StringToNum            1
//  Dec2Str                3    PStr2Dec               2
// 00000034: A9EB            '..'            _FP68K                       ; A9EB
//  all kinds of floatinig point operations
//  FOABS                 15    FOLOGB                26    FOGETENV               3
//  FOADD                  0    FOMUL                  4    FOSETHV                5
//  FOB2D                 11    FONEG                 13    FOSETXCP              21
//  FOCLASS               28    FONEXT                19    FOSQRT                18
//  FOCMP                  8    FOPROCENTRY           23    FOSUB                  2
//  FOCPX                 10    FOPROCEXIT            25    FOTESTXCP             27
//  FOCPYSGN              17    FOREM                 12    FOTTI                 22
//  FOD2B                  9    FORTI                 20    FOX2Z                 16
//  FODIV                  6    FOSCALB               24    FOZ2X                 14
//  FOGETHV                7    FOSETENV               1
// 00000250: A9C9            '..'            _SysError                     ; A9C9
//  Set the system error code
// 0000A41A: A023            '.#'            _DisposeHandle                     ; A023
//  handle, free memory and the master pointer, MemError
// 0000A49E: A128            '.('            _RecoverHandle                     ; A128
//  ptr, returns handle, creates a handle by finding the Master Pointr to a memory block
// 0001D8DC: A021            '.!'            _GetPtrSize                     ; A021
//  ptr, returns size, return the logical size of a non-relocatable block
// 0001D8EA: A025            '.%'            _GetHandleSize                     ; A025
//  handle, returns size, return the logical size of a relocatable block
// 0001DA66: A9A2            '..'            _LoadResource                     ; A9A2
//  handle, returns error?
// 0001DAAA: A9F1            '..'            _UnLoadSeg                     ; A9F1
//  Don't... . Used by '__CplusInit', but also patched bby the 'far' model
// 0001DAAE: A9A3            '..'            _ReleaseResource                     ; A9A3
//  handle, ResError, Releases the memory allocated by a given resource (not clear if the master pointer is released?)
// 0001DAE2: A9A1            '..'            _GetNamedResource                     ; A9A1
//  same as _Get1NamedResource
// 0001E0DC: A851            '.Q'            _SetCursor                     ; A851
//  Changes the mouse pointer shape
// 000000B6: A198            '..'            _HWPriv                      ; A198
//  uses subcalls in D0: 14, 15, 16 = NuBus card cache management
//  we use 0x01 in 'flush_cache', but only if this call is implemented at all
// 00000422: A9FF            '..'            _Debugger                    ; A9FF
//  jump into ROM debugger or whatever is there (moste severe error for the user next to a crash!)

// 0000A28E: A88F            '..'            _OSDispatch                     ; A88F (1d, 20)
// #define mfMaxMemSel		21
// #define mfFreeMemSel		24
// #define mfTempNewHandleSel	29 = 0x1d
// #define mfTempHLockSel		30
// #define mfTempHUnLockSel	31
//  #define mfTempDisposHandleSel	32 = 0x20
// 0x37:
// 0x3A:


// 7.0004BA: dc.w    $a02e; opcode 1010 (_BlockMove)
//      case 0xa9a0: mosTrapGetResource(instr); break; // TODO: unverified



/*
 Unix hooks;
 0000003C: 4E56 0000       'NV..'          LINK.W   A6,#$0000
 00000040: 4E5E            'N^'            UNLK     A6
 00000042: 4E75            'Nu'            RTS
 
 1.0007F6: move.l  (-$3c,A5), -(A7)  envp
 1.0007FA: move.l  (-$40,A5), -(A7)  argv
 1.0007FE: move.l  (-$44,A5), -(A7)  argc
*/

/*
 Mem 0x0316
 
 0x0316: ptr to MPW_Hdl
 
 MPW_Hdl:   'MPGM'
 MPW_Hdl+4: Ptr to MPW_Mem
 
 MPW_Mem:      w: ??, must not be 0
 MPW_Mem+0x02: l: argc
 MPW_Mem+0x06: l: argv
 MPW_Mem+0x0a: l: envp
 ...
 MPW_Mem+0x20: l: ??
 MPW_Mem+0x24: Ptr (address of global -$022C(A5))
 
 errno, McOSErr

 
 _initIOPtable:
 00000488:
*/

/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ioctl.h	1.4 (2.11BSD GTE) 1997/3/28
 */

/*
 * Ioctl definitions
 */
#ifndef	_IOCTL_
#define _IOCTL_
#ifdef KERNEL
#include "ttychars.h"
#include "ttydev.h"
#else
#include <sys/ttychars.h>
#include <sys/ttydev.h>

int     ioctl (int d, int request, ...);

#endif

struct tchars {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};
struct ltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};

/*
 * Structure for TIOCGETP and TIOCSETP ioctls.
 */

#ifndef _SGTTYB_
#define _SGTTYB_
struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	short	sg_flags;		/* mode flags */
};
#endif

/*
 * Window/terminal size structure.
 * This information is stored by the kernel
 * in order to provide a consistent interface,
 * but is not used by the kernel.
 *
 * Type must be "unsigned short" so that types.h not required.
 */
struct winsize {
	unsigned short	ws_row;			/* rows, in characters */
	unsigned short	ws_col;			/* columns, in characters */
	unsigned short	ws_xpixel;		/* horizontal size, pixels */
	unsigned short	ws_ypixel;		/* vertical size, pixels */
};

/*
 * Pun for SUN.
 */
struct ttysize {
	unsigned short	ts_lines;
	unsigned short	ts_cols;
	unsigned short	ts_xxx;
	unsigned short	ts_yyy;
};
#define TIOCGSIZE	TIOCGWINSZ
#define TIOCSSIZE	TIOCSWINSZ

#ifndef _IO
/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 256 bytes (disklabels are 216 bytes).
 */
#define IOCPARM_MASK	0xff		/* parameters must be < 256 bytes */
#define IOC_VOID	0x20000000	/* no parameters */
#define IOC_OUT		0x40000000	/* copy out parameters */
#define IOC_IN		0x80000000	/* copy in parameters */
#define IOC_INOUT	(IOC_IN|IOC_OUT)

#define _IO(x,y)	(IOC_VOID |                               ((x)<<8)|y)
#define _IOR(x,y,t)	(IOC_OUT  |((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)
#define _IOW(x,y,t)	(IOC_IN   |((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)
/* this should be _IORW, but stdio got there first */
#define _IOWR(x,y,t)	(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)
#define _ION(x,y,n)	(IOC_INOUT|      (((n)&IOCPARM_MASK)<<16)|((x)<<8)|y)
#endif

/*
 * tty ioctl commands
 */
#define TIOCGETD	_IOR('t', 0, int)	/* get line discipline */
#define TIOCSETD	_IOW('t', 1, int)	/* set line discipline */
#define TIOCHPCL	_IO ('t', 2)		/* hang up on last close */
#define TIOCMODG	_IOR('t', 3, int)	/* get modem control state */
#define TIOCMODS	_IOW('t', 4, int)	/* set modem control state */
#define 	TIOCM_LE	0001		/* line enable */
#define 	TIOCM_DTR	0002		/* data terminal ready */
#define 	TIOCM_RTS	0004		/* request to send */
#define 	TIOCM_ST	0010		/* secondary transmit */
#define 	TIOCM_SR	0020		/* secondary receive */
#define 	TIOCM_CTS	0040		/* clear to send */
#define 	TIOCM_CAR	0100		/* carrier detect */
#define 	TIOCM_CD	TIOCM_CAR
#define 	TIOCM_RNG	0200		/* ring */
#define 	TIOCM_RI	TIOCM_RNG
#define 	TIOCM_DSR	0400		/* data set ready */
#define TIOCGETP	_IOR('t', 8,struct sgttyb)/* get parameters -- gtty */
#define TIOCSETP	_IOW('t', 9,struct sgttyb)/* set parameters -- stty */
#define TIOCSETN	_IOW('t', 10,struct sgttyb)/* as above, but no flushtty */
#define TIOCEXCL	_IO ('t', 13)		/* set exclusive use of tty */
#define TIOCNXCL	_IO ('t', 14)		/* reset exclusive use of tty */
#define TIOCFLUSH	_IOW('t', 16, int)	/* flush buffers */
#define TIOCSETC	_IOW('t', 17,struct tchars)/* set special characters */
#define TIOCGETC	_IOR('t', 18,struct tchars)/* get special characters */
#define 	TANDEM		0x00000001	/* send stopc on out q full */
#define 	CBREAK		0x00000002	/* half-cooked mode */
						/* 0x4 (old LCASE) */
#define 	ECHO		0x00000008	/* echo input */
#define 	CRMOD		0x00000010	/* map \r to \r\n on output */
#define 	RAW		0x00000020	/* no i/o processing */
#define 	ODDP		0x00000040	/* get/send odd parity */
#define 	EVENP		0x00000080	/* get/send even parity */
#define 	ANYP		0x000000c0	/* get any parity/send none */
						/* 0x100 (old NLDELAY) */
						/* 0x200 */
#define 	XTABS		0x00000400	/* expand tabs on output */
						/* 0x0800 (part of old XTABS) */
						/* 0x1000 (old CRDELAY) */
						/* 0x2000 */
						/* 0x4000 (old VTDELAY) */
						/* 0x8000 (old BSDELAY) */
#define 	CRTBS		0x00010000	/* do backspacing for crt */
#define 	PRTERA		0x00020000	/* \ ... / erase */
#define 	CRTERA		0x00040000	/* " \b " to wipe out char */
						/* 0x00080000 (old TILDE) */
#define 	MDMBUF		0x00100000	/* start/stop output on carrier intr */
#define 	LITOUT		0x00200000	/* literal output */
#define 	TOSTOP		0x00400000	/* SIGSTOP on background output */
#define 	FLUSHO		0x00800000	/* flush output to terminal */
#define 	NOHANG		0x01000000	/* no SIGHUP on carrier drop */
#define 	RTSCTS		0x02000000	/* use RTS/CTS flow control */
#define 	CRTKIL		0x04000000	/* kill line with " \b " */
#define 	PASS8		0x08000000
#define 	CTLECH		0x10000000	/* echo control chars as ^X */
#define 	PENDIN		0x20000000	/* tp->t_rawq needs reread */
#define 	DECCTQ		0x40000000	/* only ^Q starts after ^S */
#define 	NOFLSH		0x80000000	/* no output flush on signal */
/* locals, from 127 down */
#define TIOCLBIS	_IOW('t', 127, int)	/* bis local mode bits */
#define TIOCLBIC	_IOW('t', 126, int)	/* bic local mode bits */
#define TIOCLSET	_IOW('t', 125, int)	/* set entire local mode word */
#define TIOCLGET	_IOR('t', 124, int)	/* get local modes */
#define 	LCRTBS		((int)(CRTBS>>16))
#define 	LPRTERA		((int)(PRTERA>>16))
#define 	LCRTERA		((int)(CRTERA>>16))
#define 	LMDMBUF		((int)(MDMBUF>>16))
#define 	LLITOUT		((int)(LITOUT>>16))
#define 	LTOSTOP		((int)(TOSTOP>>16))
#define 	LFLUSHO		((int)(FLUSHO>>16))
#define 	LNOHANG		((int)(NOHANG>>16))
#define 	LRTSCTS		((int)(RTSCTS>>16))
#define 	LCRTKIL		((int)(CRTKIL>>16))
#define 	LPASS8		((int)(PASS8>>16))
#define 	LCTLECH		((int)(CTLECH>>16))
#define 	LPENDIN		((int)(PENDIN>>16))
#define 	LDECCTQ		((int)(DECCTQ>>16))
#define 	LNOFLSH		((int)(NOFLSH>>16))
#define TIOCSBRK	_IO ('t', 123)		/* set break bit */
#define TIOCCBRK	_IO ('t', 122)		/* clear break bit */
#define TIOCSDTR	_IO ('t', 121)		/* set data terminal ready */
#define TIOCCDTR	_IO ('t', 120)		/* clear data terminal ready */
#define TIOCGPGRP	_IOR('t', 119, int)	/* get pgrp of tty */
#define TIOCSPGRP	_IOW('t', 118, int)	/* set pgrp of tty */
#define TIOCSLTC	_IOW('t', 117,struct ltchars)/* set local special chars */
#define TIOCGLTC	_IOR('t', 116,struct ltchars)/* get local special chars */
#define TIOCOUTQ	_IOR('t', 115, int)	/* output queue size */
#define TIOCSTI		_IOW('t', 114, char)	/* simulate terminal input */
#define TIOCNOTTY	_IO ('t', 113)		/* void tty association */
#define TIOCPKT		_IOW('t', 112, int)	/* pty: set/clear packet mode */
#define 	TIOCPKT_DATA		0x00	/* data packet */
#define 	TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define 	TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define 	TIOCPKT_STOP		0x04	/* stop output */
#define 	TIOCPKT_START		0x08	/* start output */
#define 	TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define 	TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */
#define TIOCSTOP	_IO ('t', 111)		/* stop output, like ^S */
#define TIOCSTART	_IO ('t', 110)		/* start output, like ^Q */
#define TIOCMSET	_IOW('t', 109, int)	/* set all modem bits */
#define TIOCMBIS	_IOW('t', 108, int)	/* bis modem bits */
#define TIOCMBIC	_IOW('t', 107, int)	/* bic modem bits */
#define TIOCMGET	_IOR('t', 106, int)	/* get all modem bits */
#define TIOCREMOTE	_IOW('t', 105, int)	/* remote input editing */
#define TIOCGWINSZ	_IOR('t', 104, struct winsize)	/* get window size */
#define TIOCSWINSZ	_IOW('t', 103, struct winsize)	/* set window size */
#define TIOCUCNTL	_IOW('t', 102, int)	/* pty: set/clr usr cntl mode */
#define UIOCCMD(n)	_IO ('u', n)		/* usr cntl op "n" */

#define NTTYDISC	0                       /* new tty discipline */

#define FIOCLEX		_IO('f', 1)		/* set exclusive use on fd */
#define FIONCLEX	_IO('f', 2)		/* remove exclusive use */
/* another local */
/* should use off_t for FIONREAD but that would require types.h */
#define FIONREAD	_IOR('f', 97, long)	/* get # bytes to read */
#define FIONBIO		_IOW('f', 96, int)	/* set/clear non-blocking i/o */
#define FIOASYNC	_IOW('f', 95, int)	/* set/clear async i/o */
#define FIOSETOWN	_IOW('f', 94, int)	/* set owner */
#define FIOGETOWN	_IOR('f', 93, int)	/* get owner */

/* socket i/o controls */
#define SIOCSHIWAT	_IOW('s', 0, int)		/* set high watermark */
#define SIOCGHIWAT	_IOR('s', 1, int)		/* get high watermark */
#define SIOCSLOWAT	_IOW('s', 2, int)		/* set low watermark */
#define SIOCGLOWAT	_IOR('s', 3, int)		/* get low watermark */
#define SIOCATMARK	_IOR('s', 7, int)		/* at oob mark? */
#define SIOCSPGRP	_IOW('s', 8, int)		/* set process group */
#define SIOCGPGRP	_IOR('s', 9, int)		/* get process group */

#define SIOCADDRT	_IOW('r', 10, struct rtentry)	/* add route */
#define SIOCDELRT	_IOW('r', 11, struct rtentry)	/* delete route */

#define SIOCSIFADDR	_IOW ('i',12, struct ifreq)	/* set ifnet address */
#define SIOCGIFADDR	_IOWR('i',13, struct ifreq)	/* get ifnet address */
#define SIOCSIFDSTADDR	_IOW ('i',14, struct ifreq)	/* set p-p address */
#define SIOCGIFDSTADDR	_IOWR('i',15, struct ifreq)	/* get p-p address */
#define SIOCSIFFLAGS	_IOW ('i',16, struct ifreq)	/* set ifnet flags */
#define SIOCGIFFLAGS	_IOWR('i',17, struct ifreq)	/* get ifnet flags */
#define SIOCGIFBRDADDR	_IOWR('i',18, struct ifreq)	/* get broadcast addr */
#define SIOCSIFBRDADDR	_IOW ('i',19, struct ifreq)	/* set broadcast addr */
#define SIOCGIFCONF	_IOWR('i',20, struct ifconf)	/* get ifnet list */
#define SIOCGIFNETMASK	_IOWR('i',21, struct ifreq)	/* get net addr mask */
#define SIOCSIFNETMASK	_IOW ('i',22, struct ifreq)	/* set net addr mask */
#define SIOCGIFMETRIC	_IOWR('i',23, struct ifreq)	/* get IF metric */
#define SIOCSIFMETRIC	_IOW ('i',24, struct ifreq)	/* set IF metric */

#define SIOCSARP	_IOW ('i',30, struct arpreq)	/* set arp entry */
#define SIOCGARP	_IOWR('i',31, struct arpreq)	/* get arp entry */
#define SIOCDARP	_IOW ('i',32, struct arpreq)	/* delete arp entry */

#endif

#define	EPERM		1		/* Not owner */
#define	ENOENT		2		/* No such file or directory */
#define	ESRCH		3		/* No such process */
#define	EINTR		4		/* Interrupted system call */
#define	EIO		5		/* I/O error */
#define	ENXIO		6		/* No such device or address */
#define	E2BIG		7		/* Arg list too long */
#define	ENOEXEC		8		/* Exec format error */
#define	EBADF		9		/* Bad file number */
#define	ECHILD		10		/* No children */
#define	EAGAIN		11		/* No more processes */
#define	ENOMEM		12		/* Not enough core */
#define	EACCES		13		/* Permission denied */
#define	EFAULT		14		/* Bad address */
#define	ENOTBLK		15		/* Block device required */
#define	EBUSY		16		/* Mount device busy */
#define	EEXIST		17		/* File exists */
#define	EXDEV		18		/* Cross-device link */
#define	ENODEV		19		/* No such device */
#define	ENOTDIR		20		/* Not a directory*/
#define	EISDIR		21		/* Is a directory */
#define	EINVAL		22		/* Invalid argument */
#define	ENFILE		23		/* File table overflow */
#define	EMFILE		24		/* Too many open files */
#define	ENOTTY		25		/* Not a typewriter */
#define	ETXTBSY		26		/* Text file busy */
#define	EFBIG		27		/* File too large */
#define	ENOSPC		28		/* No space left on device */
#define	ESPIPE		29		/* Illegal seek */
#define	EROFS		30		/* Read-only file system */
#define	EMLINK		31		/* Too many links */
#define	EPIPE		32		/* Broken pipe */

/* math software */
#define	EDOM		33		/* Argument too large */
#define	ERANGE		34		/* Result too large */

/* non-blocking and interrupt i/o */
#define	EWOULDBLOCK	35		/* Operation would block */
#define	EDEADLK		EWOULDBLOCK	/* ditto */
#define	EINPROGRESS	36		/* Operation now in progress */
#define	EALREADY	37		/* Operation already in progress */

/* ipc/network software */

	/* argument errors */
#define	ENOTSOCK	38		/* Socket operation on non-socket */
#define	EDESTADDRREQ	39		/* Destination address required */
#define	EMSGSIZE	40		/* Message too long */
#define	EPROTOTYPE	41		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	42		/* Protocol not available */
#define	EPROTONOSUPPORT	43		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define	EOPNOTSUPP	45		/* Operation not supported on socket */
#define	EPFNOSUPPORT	46		/* Protocol family not supported */
#define	EAFNOSUPPORT	47		/* Address family not supported by protocol family */
#define	EADDRINUSE	48		/* Address already in use */
#define	EADDRNOTAVAIL	49		/* Can't assign requested address */

	/* operational errors */
#define	ENETDOWN	50		/* Network is down */
#define	ENETUNREACH	51		/* Network is unreachable */
#define	ENETRESET	52		/* Network dropped connection on reset */
#define	ECONNABORTED	53		/* Software caused connection abort */
#define	ECONNRESET	54		/* Connection reset by peer */
#define	ENOBUFS		55		/* No buffer space available */
#define	EISCONN		56		/* Socket is already connected */
#define	ENOTCONN	57		/* Socket is not connected */
#define	ESHUTDOWN	58		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	59		/* Too many references: can't splice */
#define	ETIMEDOUT	60		/* Connection timed out */
#define	ECONNREFUSED	61		/* Connection refused */

	/* */
#define	ELOOP		62		/* Too many levels of symbolic links */
#define	ENAMETOOLONG	63		/* File name too long */

/* should be rearranged */
#define	EHOSTDOWN	64		/* Host is down */
#define	EHOSTUNREACH	65		/* No route to host */
#define	ENOTEMPTY	66		/* Directory not empty */

/* quotas & mush */
#define	EPROCLIM	67		/* Too many processes */
#define	EUSERS		68		/* Too many users */
#define	EDQUOT		69		/* Disc quota exceeded */

/* Network File System */
#define	ESTALE		70		/* Stale NFS file handle */
#define	EREMOTE		71		/* Too many levels of remote in path */
#define	EBADRPC		72		/* RPC struct is bad */
#define	ERPCMISMATCH	73		/* RPC version wrong */
#define	EPROGUNAVAIL	74		/* RPC prog. not avail */
#define	EPROGMISMATCH	75		/* Program version wrong */
#define	EPROCUNAVAIL	76		/* Bad procedure for program */

#define	ENOLCK		77		/* No locks available */
#define	ENOSYS		78		/* Function not implemented */

#define	EFTYPE		79		/* Inappropriate file type or format */
#define	EAUTH		80		/* Authentication error */
#define	ENEEDAUTH	81		/* Need authenticator */
#define	ELAST		81		/* Must be equal largest errno */


 *            O_RDONLY      0      Open for read
 *            O_WRONLY      1      Open for write
 *            O_RDWR        2      Open for read and write
 *            O_CREAT       4      Create file if it doesn't exist
 *            O_EXCL        8      With O_CREAT, fail if file already exists
 *            O_TRUNC      16      Truncate file upon open
 *            O_APPEND     32      All writes happen at EOF (optional feature)

