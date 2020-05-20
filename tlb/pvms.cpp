#include "pvms.h"

//TLBEntry::TLBEntry() : valid(0), vpn(0), pfn(0) {}

TLB_type::TLB_type (int num) : numEntries(num), repIndex(0) {
	table.resize(numEntries);
}

int TLB_type::checkTLB(unsigned int vpn) {
	for (int i = 0; i < numEntries; i++) {
		if ((table.at(i).valid == 1) && (table.at(i).vpn == vpn)) {
			return i;
		}
	}
	return -1;
}

PT_type::PT_type() : addrSize(65536) {
	pageTable = new PageTableEntry[addrSize];
	for (int i = 0; i < addrSize; i++) {
		pageTable[i].presence = 0;
	}
}

CoreMap_type::CoreMap_type(int num) : pageFrames(num) {
	map = new CoreMapEntry[pageFrames];
	for (int i = 0; i < pageFrames; i++) {
		map[i].valid = 0;
		map[i].use_vector = 0;
	}
}

int CoreMap_type::checkMap() {
	for (int i = 0; i < pageFrames; i++) {
		if (map[i].valid == 0) {
			return i;
		}
	}
	return -1;
}

int CoreMap_type::frameRep() {
	unsigned char min = 255;
	int repIndex = 0;
	for (int i = 0; i < pageFrames; i++) {
		if (map[i].use_vector < min) {
			repIndex = i;
			min = map[i].use_vector;
		}
	}
	return repIndex;
}

void CoreMap_type::usedFrame(int pfn) {
	map[pfn].use_vector = map[pfn].use_vector | 0x80;
}
