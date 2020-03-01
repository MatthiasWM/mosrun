/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013  Matthias Melcher
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 The author can be contacted at mosrun AT matthiasm DOT com.
 The latest source code can be found at http://code.google.com/p/dynee5/
 */

#include "log.h"

#include <stdarg.h>


// Inlcude Musahi's m68k emulator

extern "C" {
#include "musashi331/m68k.h"
#include "musashi331/m68kcpu.h"
#include "musashi331/m68kops.h"
}


static FILE *gMosLogFile = stderr;
static int gMosVerbosity = MOS_VERBOSITY_WARN;


void mosLogVerbosity(int v)
{
  gMosVerbosity = v;
}


int mosLogVerbosity()
{
  return gMosVerbosity;
}


/**
 * Return the file struct to which we log our messages.
 */
FILE *mosLogFile()
{
  return gMosLogFile;
}


/**
 * Set the destination file for logging.
 * This call does not create or close any file handles.
 */
void mosLogTo(FILE *out)
{
  gMosLogFile = out;
}


void mosLogClose()
{
  if (gMosLogFile && gMosLogFile!=stdout && gMosLogFile!=stderr) {
    fclose(gMosLogFile);
  }
}


/**
 * Log text to a file using print() syntax.
 *
 * Use this for any and every little thing that goes on in the simulator
 * or emulator. This option may generate megabytes of text.
 */
void mosTrace(const char *format, ...)
{
  if (gMosVerbosity<MOS_VERBOSITY_TRACE)
    return;
  if (gMosLogFile) {
    va_list va;
    va_start(va, format);
    // TODO: vsnprintf
    vfprintf(gMosLogFile, format, va);
    va_end(va);
  }
}


/**
 * Log text to a file using print() syntax.
 *
 * Use this for any uncommon events that may be of interest for the programmer.
 */
void mosDebug(const char *format, ...)
{
  if (gMosVerbosity<MOS_VERBOSITY_DEBUG)
    return;
  if (gMosLogFile) {
    va_list va;
    va_start(va, format);
    // TODO: vsnprintf
    vfprintf(gMosLogFile, format, va);
    va_end(va);
  }
}


/**
 * Log text to a file using print() syntax.
 *
 * Log events that mey be useful for the common user.
 */
void mosLog(const char *format, ...)
{
  if (gMosVerbosity<MOS_VERBOSITY_LOG)
    return;
  if (gMosLogFile) {
    va_list va;
    va_start(va, format);
    // TODO: vsnprintf
    vfprintf(gMosLogFile, format, va);
    va_end(va);
  }
}


/**
 * Print text to stderr about issues with the emulation.
 *
 * Log events that are important for the user, but not catastrophic for the 
 * application. This is the defaul log level.
 */
void mosWarning(const char *format, ...)
{
  if (gMosVerbosity<MOS_VERBOSITY_WARN)
    return;
  va_list va;
  va_start(va, format);
  fprintf(gMosLogFile, "MOSRUN - WARNING!\n");
  vfprintf(gMosLogFile, format, va);
  va_end(va);
  
  if (gMosLogFile!=stderr) {
    va_start(va, format);
    fprintf(stderr, "MOSRUN - WARNING!\n");
    vfprintf(stderr, format, va);
    va_end(va);
  }
}


/**
 * Print text to stderr about severe errors in the meulation.
 *
 * Log events that will abort the application immediatly, for example 
 * unemulated traps.
 * After calling mosError, the application should safely exit.
 */
void mosError(const char *format, ...)
{
  if (gMosVerbosity<MOS_VERBOSITY_ERR)
    return;
  va_list va;
  va_start(va, format);
  fprintf(gMosLogFile, "MOSRUN - ERROR!\n");
  vfprintf(gMosLogFile, format, va);
  va_end(va);
  
  if (gMosLogFile!=stderr) {
    va_start(va, format);
    fprintf(stderr, "MOSRUN - ERROR!\n");
    vfprintf(stderr, format, va);
    va_end(va);
  }
}

void mosTraceRegisters()
{
    mosTrace("D0:%08X D1:%08X D2:%08X D3:%08X D4:%08X D5:%08X D6:%08X D7:%08X\n",
             m68k_get_reg(0L, M68K_REG_D0),
             m68k_get_reg(0L, M68K_REG_D1),
             m68k_get_reg(0L, M68K_REG_D2),
             m68k_get_reg(0L, M68K_REG_D3),
             m68k_get_reg(0L, M68K_REG_D4),
             m68k_get_reg(0L, M68K_REG_D5),
             m68k_get_reg(0L, M68K_REG_D6),
             m68k_get_reg(0L, M68K_REG_D7)
             );
    mosTrace("A0:%08X A1:%08X A2:%08X A3:%08X A4:%08X A5:%08X A6:%08X A7:%08X\n",
             m68k_get_reg(0L, M68K_REG_A0),
             m68k_get_reg(0L, M68K_REG_A1),
             m68k_get_reg(0L, M68K_REG_A2),
             m68k_get_reg(0L, M68K_REG_A3),
             m68k_get_reg(0L, M68K_REG_A4),
             m68k_get_reg(0L, M68K_REG_A5),
             m68k_get_reg(0L, M68K_REG_A6),
             m68k_get_reg(0L, M68K_REG_A7)
             );
}

