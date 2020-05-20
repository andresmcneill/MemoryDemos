#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <cstdio>
#include <stdlib.h>
#ifndef PVMS_H
#define PVMS_H

class PageTableEntry {
public:
    unsigned char presence;   /* could be single bit */
    unsigned short pad;
    unsigned char pfn;        /* 8 bits */
};

class TLBEntry {
public:
    unsigned char valid;      /* could be single bit */
    unsigned short vpn;       /* 16 bits */
    unsigned char pfn;        /* 8 bits */
	//TLBEntry();
};

class CoreMapEntry {
public:
    unsigned char valid;      /* could be single bit */
    unsigned char use_vector; /* 8 bits for pseudo-LRU replacement */
    unsigned short vpn;       /* 16 bits */
};

class TLB_type {
public:
	//	returns index of TLB entry in "table" on hit, else returns -1 on miss
	int checkTLB(unsigned int vpn);
	TLB_type(int num);
    int numEntries;
	std::vector<TLBEntry> table;
    int repIndex;
};

class PT_type {
private:
	//	Size of virtual address space, 2^16 entries for 16 bit virtual address
	int addrSize;
public:
	PT_type();
	PageTableEntry *pageTable;
};

class CoreMap_type {
public:
    int checkMap();
    int pageFrames;
    CoreMap_type(int num);
    void usedFrame(int pfn);
    int frameRep();
    CoreMapEntry *map;
};

#endif
