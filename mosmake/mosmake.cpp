//
//  main.cpp
//  mosrun
//
//  Created by Matthias Melcher on 14.07.13.
//  Copyright (c) 2013 Matthias Melcher. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/xattr.h>
#include <fcntl.h>

typedef unsigned char byte;

byte *theApp = 0;
ssize_t theAppSize;
byte *theRsrc = 0;
ssize_t theRsrcSize;

const char *c_filename = "/Users/matt/dev/Alienate/mosrun/rsrc.cpp";
const char *h_filename = "/Users/matt/dev/Alienate/mosrun/rsrc.h";


void saveDefaultCode()
{
  FILE *c_file = fopen(c_filename, "wb");
  fputs(
        "\n"
        "#include \"rsrc.h\"\n"
        "\n"
        "unsigned char *gAppResource = 0L;\n"
        "unsigned int gAppResourceSize = 0;\n"
        "\n",
        c_file);
  fclose(c_file);
  FILE *h_file = fopen(h_filename, "wb");
  fputs(
        "\n"
        "#ifndef __mosrun__rsrc__\n"
        "#define __mosrun__rsrc__\n"
        "\n"
        "extern unsigned char *gAppResource;\n"
        "extern unsigned int gAppResourceSize;\n"
        "\n"
        "#endif /* defined(__mosrun__rsrc__) */\n"
        "\n",
        h_file);
  fclose(h_file);
}


void saveCode()
{
  int i;
  FILE *c_file = fopen(c_filename, "wb");
  fputs(
        "\n"
        "#include \"rsrc.h\"\n"
        "\n"
        "unsigned char rsrc[] = {\n",
        c_file);
  for (i=0; i<theAppSize; i++) {
    if ((i&15)==0) fprintf(c_file, "    ");
    fprintf(c_file, "0x%02X, ", theApp[i]);
    if ((i&15)==15) fprintf(c_file, "\n");
  }
  fputs(
        "\n};\n"
        "\n"
        "unsigned char *gAppResource = rsrc;\n"
        "unsigned int gAppResourceSize = sizeof(rsrc);\n"
        "\n",
        c_file);
  fclose(c_file);
  FILE *h_file = fopen(h_filename, "wb");
  fputs(
        "\n"
        "#ifndef __mosrun__rsrc__\n"
        "#define __mosrun__rsrc__\n"
        "\n"
        "extern unsigned char *gAppResource;\n"
        "extern unsigned int gAppResourceSize;\n"
        "\n"
        "#endif /* defined(__mosrun__rsrc__) */\n"
        "\n",
        h_file);
  fclose(h_file);
}


int loadApp(const char *aName)
{
  ssize_t size = getxattr(aName, "com.apple.ResourceFork", 0L, 0, 0, 0);
  if (size==-1) {
    return 0;
  }
  theAppSize = size;
  theApp = (byte*)calloc(1, size);
  ssize_t ret = getxattr(aName, "com.apple.ResourceFork", theApp, size, 0, 0);
  if (ret==-1) {
    return 0;
  }
  return 1;
}


unsigned int m68k_read_memory_8(unsigned int address)
{
  return *((unsigned char*)address);
}

unsigned int m68k_read_memory_16(unsigned int address)
{
  return ntohs(*((unsigned short*)address));
}


unsigned int m68k_read_memory_32(unsigned int address)
{
  return ntohl(*((unsigned long*)address));
}


int main(int argc, const char * argv[])
{
  if (argv[1]) {
    loadApp(argv[1]);
    saveCode();
  } else {
    saveDefaultCode();
  }
  return 0;
}




