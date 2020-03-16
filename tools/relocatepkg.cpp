/*
 relocatepkg - Change the base address of a .pkg file in memory
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unordered_set>


static uint32_t gOldAddr = 0;
static uint32_t gNewAddr = 0;
static std::unordered_set<uint32_t> gVisited;
static std::unordered_set<uint32_t> gRelocated;

extern int relocateNOSObject(uint8_t *pkg, uint32_t objAddr);

/**
 Read an uint32_t value from the package.
 */
uint32_t PkgReadU32(uint8_t *d)
{
    return ((d[0]<<24)|(d[1]<<16)|(d[2]<<8)|(d[3]));
}

/**
 Write an uint32_t value to the package.
 */
void PkgWriteU32(uint8_t *d, uint32_t v)
{
    d[0] = v>>24;
    d[1] = v>>16;
    d[2] = v>>8;
    d[3] = v;
}

int relocateNOSRef(uint8_t *pkg, uint32_t refAddr)
{
    uint32_t ref = PkgReadU32(pkg+refAddr);
    uint32_t objAddr;

    switch (ref&3) {
        case 0: // integer
            break;
        case 1: // pointer
            // check if this address was already relocated into the new address space
            if (gRelocated.find(refAddr)==gRelocated.end()) {
                objAddr = ref-gOldAddr;
                PkgWriteU32(pkg+refAddr, objAddr+gNewAddr);
                gRelocated.insert(refAddr);
            } else {
                objAddr = ref-gNewAddr;
            }
            // check if we already visited the original address
            if (gVisited.find(objAddr)==gVisited.end()) {
                gVisited.insert(objAddr);
                relocateNOSObject(pkg, objAddr&~3);
            }
            break;
        case 2: // special
            break;
        case 3: // magic pointer
            break;
    }
    return 0;
}

int relocateNOSBinaryObject(uint8_t *pkg, uint32_t objAddr)
{
    relocateNOSRef(pkg, objAddr+8); // class
    return 0;
}

int relocateNOSArray(uint8_t *pkg, uint32_t objAddr)
{
    uint32_t size = PkgReadU32(pkg+objAddr) >> 8;
    uint32_t num_slots = size/4 - 3;
    uint32_t i;

    relocateNOSRef(pkg, objAddr+8); // class
    for (i=0; i<num_slots; i++) {
        relocateNOSRef(pkg, objAddr+12 + 4*i);
    }
    return 0;
}

int relocateNOSFrame(uint8_t *pkg, uint32_t objAddr)
{
    uint32_t size = PkgReadU32(pkg+objAddr) >> 8;
    uint32_t num_slots = size/4 - 3;
    uint32_t i;

    relocateNOSRef(pkg, objAddr+8); // map
    for (i=0; i<num_slots; i++) {
        relocateNOSRef(pkg, objAddr+12 + 4*i);
    }
    return 0;
}

int relocateNOSObject(uint8_t *pkg, uint32_t objAddr)
{
    uint32_t obj = PkgReadU32(pkg+objAddr);
    switch (obj & 0xff) {
        case 0x40: // binary object
            relocateNOSBinaryObject(pkg, objAddr);
            break;
        case 0x41: // array
            relocateNOSArray(pkg, objAddr);
            break;
        case 0x43: // frame
            relocateNOSFrame(pkg, objAddr);
            break;
        default:
            printf("RelocatePkg: WARNING: unknown object type.\n");
            break;
    }
    return 0;
}

/**
 Relocate a Newton Script part by the given number of bytes.

 \param pkg address of the package image in memory
 \param part start of the part data
 \param delta move the pointer by this amount of bytes

 \return -1 if this function failed.
 */
int relocateNOSPart(uint8_t *pkg, uint32_t partData)
{
    if (PkgReadU32(pkg+partData)!=0x00001041 || PkgReadU32(pkg+partData+8)!=0x00000002) {
        printf("RelocatePkg: package data not found.\n");
        return -1;
    }
    // offset to the first object at oldAddress in 4-byte alignment
    relocateNOSRef(pkg, partData+12);
    return 0;
}

/**
 Takes any .pkg file in RAM and relocates it to a new position in ROM.

 If you are a true Newton geek and crazy developer who likes to go into the guts of the NewtonOS,
 you may have been aware that the Newton .pkg file format looks surprisingly similar to the representation
 of a packae in memory. Everything except pointers are ready to run the code in place. The pointers
 within the package though give the offset of an address to the start of the file, because a package
 could be loaded anywhere into RAM, and then these offsets need to be converted into pointers again.

 Not so with packages that are inside a ROM Extension (REx). They need to be available right
 at boot time, and their location in ROM is well known at creation time. So even though NewtonOS
 expects a standard and fully formed package in the REx, it also expects the pointers to be pointing at
 the correct addresses in ROM.

 Now, with the internal format of the package well documented, we can find all pointers and add
 an offset to them, to point at the final address, or remove that offset, to make a package that we
 extracted from a REx back into a zero-offset package that can be loaded into memory
 by NewtonOS.

 \param pkg the address of the package in memory
 \param oldAddress the address at which the package was supposed to be in ROM, or 0 for a 'normal' package.
 \param newAddress the new position in ROM, or 0 for a package that is not in a REx

 \return -1 if anything failed, error text will be printed to the console
 */
int relocatePkg(uint8_t *pkg, uint32_t oldAddress, uint32_t newAddress)
{
    gVisited.clear();
    gRelocated.clear();

    if (oldAddress==newAddress)
        return 0;

    gOldAddr = oldAddress;
    gNewAddr = newAddress;

    if (strncmp((char*)pkg, "package", 7)!=0) {
        printf("RelocatePkg: address does not point to a valid package.\n");
        return -1;
    }
    
    uint32_t pkgFlags = PkgReadU32(pkg+12);
    if (pkgFlags&0x04000000) {
        printf("RelocatePkg: ERROR: relocation block not supported yet.\n");
        return -1;
    }
    uint32_t dirSize = PkgReadU32(pkg+44);
    uint32_t numParts = PkgReadU32(pkg+48);

    for (uint32_t i=0; i<numParts; i++) {
        uint32_t partHeader = 52 + i*32;
        uint32_t partOffset = PkgReadU32(pkg+partHeader+0);
        uint32_t partData = dirSize + partOffset /* + relocationSize */;
        uint32_t partFlags = PkgReadU32(pkg+partHeader+20);
        if ( (partFlags&0x0F) == 0x01) {
            int ret = relocateNOSPart(pkg, partData);
            if (ret==-1) return -1;
        } else {
            printf("RelocatePkg: WARNING: unknown part type in package.\n");
        }
    }

    gVisited.clear();
    gRelocated.clear();

    return 0;
}


