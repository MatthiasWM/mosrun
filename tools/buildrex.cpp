/*
 buildrex - Assemble multiple packages into a NewtonOS Rex file.
 Copyright (C) 2013-2020  Matthias Melcher

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
 The latest source code can be found at https://github.com/MatthiasWM/mosrun
 */

/**
TODO:
- output REx blocks in same order they are read from config file; don't always
  put packages at the end
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#ifdef __linux__
#include <arpa/inet.h>
#endif


uint32_t cfgId = 1;
uint32_t cfgVersion = 1;
uint32_t cfgStart = 0x00800000;
char *cfgManufacturer = strdup("Eins");
uint32_t nBlocks = 0;
class RexBlock *rexBlock[100];
uint32_t nPackages = 0;
class RexPackage *rexPackage[100];

extern int relocatePkg(uint8_t *pkg, uint32_t oldAddress, uint32_t newAddress);


class RexPackage
{
public:
    RexPackage(const char *line) {
        const char *fn = strchr(line, '"');
        if (!fn) {
            printf("BuildRex: can't find tag for package in \"%s\"\n", line);
            return;
        } else {
            pFilename = strdup(fn+1);
            int n = strlen(pFilename);
            if (pFilename[n-1]=='"')
                pFilename[n-1] = 0;
        }
        if (pFilename) {
            FILE *f = ::fopen(pFilename, "rb");
            if (!f) {
                printf("BuildRex: can't open package \"%s\" for reading\n", pFilename);
                pFilename = nullptr;
            } else {
                ::fseek(f, 0, SEEK_END);
                pSize = (uint32_t)::ftell(f);
                ::fclose(f);
            }
        }
    }
    int write(FILE *d, uint32_t offset=0xFFFFFFFF) {
        int ret = 0;
        printf("BuildRex: adding \"%s\"...\n", pFilename);
        FILE *s = ::fopen(pFilename, "rb");
        if (!s) {
            printf("BuildRex: can't open package \"%s\" for reading\n", pFilename);
            return -1;
        }
        void *data = malloc(pSize);
        ::fread(data, pSize, 1, s);
        if (offset!=0xFFFFFFFF)
            ret = relocatePkg((uint8_t*)data, 0, offset);
        ::fwrite(data, pSize, 1, d);
        ::fclose(s);
        return ret;
    }
    char *pFilename = nullptr;
    uint32_t pSize = 0;
};


class RexBlock : public RexPackage
{
public:
    RexBlock(const char *line)
    : RexPackage(line)
    {
        if (!pFilename) return;
        const char *q = strchr(line, '\'');
        if (!q) {
            printf("BuildRex: can't find tag for block in \"%s\"\n", line);
            return;
        } else {
            memcpy(&pTag, q+1, 4);
        }
    }
    uint32_t pTag = 0;
};


void writeUInt32(FILE *r, uint32_t d)
{
    uint32_t v = htonl(d);
    ::fwrite(&v, 4, 1, r);
}

void writeHeader(FILE *r)
{
    uint32_t nEntries = nBlocks + (nPackages>0);
    uint32_t offset = 40 + 12*nEntries;

    uint32_t blocksSize = 0;
    for (uint32_t i=0; i<nBlocks; i++) {
        blocksSize += rexBlock[i]->pSize;
    }

    uint32_t pkgsSize = 0;
    for (uint32_t i=0; i<nPackages; i++) {
        printf("Pkg %d: %d bytes\n", i, rexPackage[i]->pSize);
        pkgsSize += rexPackage[i]->pSize;
    }

    uint32_t totalSize = offset + blocksSize + pkgsSize;

    ::fwrite("RExBlock", 8, 1, r);
    writeUInt32(r, 0xFFFFFFFF); // checksum is undefined
    writeUInt32(r, 1); // header version
    ::fwrite(cfgManufacturer, 4, 1, r);
    writeUInt32(r, cfgVersion); // version
    writeUInt32(r, totalSize); // length of file
    writeUInt32(r, cfgId); // extension ID (0..kMaxROMExtensions-1)
    writeUInt32(r, cfgStart); // virtual address of the top of this block
    writeUInt32(r, nEntries); // number of config entries

    for (uint32_t i=0; i<nBlocks; i++) {
        RexBlock *b = rexBlock[i];
        ::fwrite(&b->pTag, 4, 1, r);
        writeUInt32(r, offset);
        writeUInt32(r, b->pSize);
        offset += b->pSize;
    }

    if (nPackages>0) {
        ::fwrite("pkgl", 4, 1, r);
        writeUInt32(r, offset);
        writeUInt32(r, pkgsSize);
    }

    for (uint32_t i=0; i<nBlocks; i++) {
        rexBlock[i]->write(r);
    }

    for (uint32_t i=0; i<nPackages; i++) {
        uint32_t pkgAddress = cfgStart + (uint32_t)::ftell(r);
        rexPackage[i]->write(r, pkgAddress);
    }
}

int buildrex(const char *cfgFile, const char *rexFile)
{
    char line[2048];

    FILE *r = ::fopen(rexFile, "wb");
    if (!r) {
        printf("BuildRex: can't open rex file \"%s\" for writing:\n%s\n", rexFile, strerror(errno));
        return -1;
    }

    FILE *f = ::fopen(cfgFile, "rb");
    if (!f) {
        printf("BuildRex: can't open config file \"%s\" for reading:\n%s\n", cfgFile, strerror(errno));
        return -1;
    }
    for (;;) {
        if (feof(f))
            break;
        line[0] = 0;
        fgets(line, sizeof(line), f);
        int n = strlen(line);
        if (line[n-1]=='\n') line[n-1] = 0;
        if (line[0]==0 || strncmp(line, "// ", 3)==0) {
            // skip
        } else if (strncmp(line, "id ", 3)==0) {
            if (sscanf(line+3, "%d", &cfgId)!=1) {
                printf("BuildRex: can't read config ID from \"%s\"\n", line);
                return -1;
            }
        } else if (strncmp(line, "version ", 8)==0) {
            if (sscanf(line+8, "%d", &cfgVersion)!=1) {
                printf("BuildRex: can't read config version from \"%s\"\n", line);
                return -1;
            }
        } else if (strncmp(line, "start ", 6)==0) {
            if (sscanf(line+5, " 0x%08X", &cfgStart)!=1) {
                printf("BuildRex: can't read config start from \"%s\"\n", line);
                return -1;
            }
        } else if (strncmp(line, "manufacturer ", 13)==0) {
            char *b = strchr(line+13, '\'');
            if (b==0) {
                printf("BuildRex: can't read config manufacturer from \"%s\"\n", line);
                return -1;
            }
            cfgManufacturer = strdup(b+1);
            if (strlen(cfgManufacturer)>4) cfgManufacturer[4] = 0;
        } else if (strncmp(line, "block ", 6)==0) { // flowed by 'xxxx' "filename"
            rexBlock[nBlocks] = new RexBlock(line);
            if (rexBlock[nBlocks]->pFilename==nullptr) return -1;
            nBlocks++;
        } else if (strncmp(line, "package ", 8)==0) { // flowed by "filename"
            rexPackage[nPackages] = new RexPackage(line);
            if (rexPackage[nPackages]->pFilename==nullptr) return -1;
            nPackages++;
        } else {
            printf("BuildRex: unknown command in config file: \"%s\"\n", line);
        }
    }

    writeHeader(r);
    // write config entries
    // write blocks

    ::fclose(f);
    ::fclose(r);
    return 0;
}


/**
 Buid a NewtonOS Rex file using a config file.
 */
int main(int argc, char **argv)
{
    if (argc!=3) {
        printf("Usage: BuildRex Einstein.rex.cfg Einstein.rex\n");
        return 30;
    } else {
        buildrex(argv[1], argv[2]);
    }
    return 0;
}
