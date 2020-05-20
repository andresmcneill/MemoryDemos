#include "pvms.h"
using namespace std;

/*	Andres McNeill
	Project 4
	cpsc 3220
	June 18, 2019

	The makefile I give should compile the program correctly. test3-test6 targets
	correspond to your trace1-trace4 test cases respectively.
*/

int main(int argc, char *argv[]) {

	int pageFrames;
	int numEntries;
	int upCount;
	bool verbose = false;

	//	Check for -v to set output to be verbose
	if (argc > 1 && (strcmp(argv[1], "-v") == 0)) {verbose = true;}

	//	Open config file
	ifstream paging;
	paging.open("paging.cfg");
	if (!paging.is_open()) {cout << "Failing to open paging.cfg" << endl;}

	//	Read config file
	string inString;
	paging >> inString;
	if (inString.compare("PF")) {cout << "Invalid config file" << endl;}
	paging >> pageFrames;
	paging >> inString;
	if (inString.compare("TE")) {cout << "Invalid config file" << endl;}
	paging >> numEntries;
	paging >> inString;
	if (inString.compare("UP")) {cout << "Invalid config file" << endl;}
	paging >> upCount;

	if (verbose) {
		cout << "paging simulation" << endl;
		cout << "  65536 virtual pages in the virtual address space" << endl;
		cout << "  " << pageFrames << " physical page frames" << endl;
		cout << "  " << numEntries << " TLB entries" << endl;
		cout << "  use vectors in core map are updated every " << upCount;
		cout << " accesses" << endl << endl;
	}

	string input;

	TLB_type TLB(numEntries);


	CoreMap_type coreMap(pageFrames);

	PT_type PT;


	unsigned int virtualAddr;
	unsigned int vpn;
	unsigned int offset;
	int tlbHit;
	int numAccess = 0;
	int numMiss = 0;
	int numFault = 0;
	unsigned int physicalAddr;
	//	Main loop, runs for each virtual address passed in from stdin
	while (cin >> input) {
		if (input.length() > 6) {
			if (input.compare("1000000") == 0) {
				cout << "statistics" << endl;
				cout << "  accesses    " << numAccess << endl;
				cout << "  tlb misses  " << numMiss << endl;
				cout << "  page faults " << numFault << endl << endl;

				cout << "tlb" << endl;
				for (int i = 0; i < numEntries; i++) {
					printf("  valid = %d, vpn = 0x%04x, pfn = 0x%02x\n",
						TLB.table[i].valid,
						TLB.table[i].vpn,
						TLB.table[i].pfn);
				}
				cout << endl;

				cout << "core map table" << endl;
				for (int i = 0; i < pageFrames; i++) {
					printf(" pfn = 0x%02x: valid = %d, use vector = 0x%02x, vpn = 0x%04x\n",
						i,
						coreMap.map[i].valid,
						coreMap.map[i].use_vector,
						coreMap.map[i].vpn);
				}
				cout << endl;

				cout << "first ten entries of page table" << endl;
				for (int i = 0; i < 10; i++) {
					printf("vpn = 0x%04x: presence = %d, pfn = 0x%02x\n",
					i,
					PT.pageTable[i].presence,
					PT.pageTable[i].pfn);
				}
				cout << endl;
			}
			else {cout << "Invalid debug value" << endl;}
		}
		else if (input.length() != 6) {cout << "Invalid 24 bit hex value\n";}
		else {
			numAccess++;
			char *cstring = new char[input.length()+1];
			strcpy(cstring, input.c_str());
			virtualAddr = strtoul(cstring, nullptr, 16);
			delete [] cstring;
			vpn = virtualAddr >> 8;
			offset = virtualAddr & 0xff;

			if (verbose) {
				cout << "access " << numAccess << ":" << endl;
				printf("  virtual address is              0x%06x\n", virtualAddr);
			}

			tlbHit = TLB.checkTLB(vpn);

			//	On TLB hit
			if (tlbHit >= 0) {
				physicalAddr = (TLB.table.at(tlbHit).pfn << 8) | offset;
				if (verbose) {printf("  tlb hit, physical address is      0x%04x\n", physicalAddr);}
				coreMap.usedFrame(TLB.table.at(tlbHit).pfn);
			}
			//	On TLB miss
			else {
				if (verbose) {cout << "  tlb miss" << endl;}
				//	On page hit
				if (PT.pageTable[vpn].presence == 1) {
					physicalAddr = (PT.pageTable[vpn].pfn << 8) | offset;
					//	update tlb
					TLB.table.at(TLB.repIndex % TLB.numEntries).valid = 1;
					TLB.table.at(TLB.repIndex % TLB.numEntries).vpn = vpn;
					TLB.table.at(TLB.repIndex % TLB.numEntries).pfn = PT.pageTable[vpn].pfn;
					coreMap.usedFrame(PT.pageTable[vpn].pfn);
					if (verbose) {
						printf("  page hit, physical address is     0x%04x\n", physicalAddr);
						printf("  tlb update of vpn 0x%04x with pfn 0x%02x\n", vpn, PT.pageTable[vpn].pfn);
					}
				}
				//	On page fault
				else {
					if (verbose) {cout << "  page fault" << endl;}
					//	If there's a free frame
					if (coreMap.checkMap() >= 0) {
						unsigned int pfn = coreMap.checkMap();
						//	update page table
						PT.pageTable[vpn].presence = 1;
						PT.pageTable[vpn].pfn = pfn;
						//	update core map
						coreMap.map[pfn].vpn = vpn;
						coreMap.map[pfn].valid = 1;
						physicalAddr = (pfn << 8) | offset;
						//	update tlb
						TLB.table.at(TLB.repIndex % TLB.numEntries).valid = 1;
						TLB.table.at(TLB.repIndex % TLB.numEntries).vpn = vpn;
						TLB.table.at(TLB.repIndex % TLB.numEntries).pfn = PT.pageTable[vpn].pfn;
						if (verbose) {
							printf("  unused page frame allocated\n");
							printf("  physical address is               0x%04x\n", physicalAddr);
							printf("  tlb update of vpn 0x%04x with pfn 0x%02x\n", TLB.table.at(TLB.repIndex % TLB.numEntries).vpn, PT.pageTable[vpn].pfn);
						}
						//	Update use vector
						coreMap.usedFrame(pfn);
					}
					//	Else must replace a frame
					else {
						if (verbose) {cout << "  page replacement needed\n";}
						int frameRep = coreMap.frameRep();
						unsigned int prevVPN = coreMap.map[frameRep].vpn;
						//	invalidate old mapping
						PT.pageTable[prevVPN].presence = 0;
						//	update page table
						PT.pageTable[vpn].presence = 1;
						PT.pageTable[vpn].pfn = (unsigned int) frameRep;
						//	update core map
						coreMap.map[frameRep].valid = 1;
						coreMap.map[frameRep].vpn = vpn;
						//	invalidate old mapping if it exists in TLB
						if (verbose) {printf("  tlb invalidate of vpn 0x%x\n", prevVPN);}
						int tlbRep = TLB.checkTLB(prevVPN);
						if (tlbRep >= 0) {
							TLB.table.at(tlbRep).valid = 0;
						}
						physicalAddr = (PT.pageTable[vpn].pfn << 8) | offset;
						//	Update use vector
						coreMap.map[frameRep].use_vector = 0;
						coreMap.usedFrame(frameRep);
						//	update tlb
						TLB.table.at(TLB.repIndex % TLB.numEntries).valid = 1;
						TLB.table.at(TLB.repIndex % TLB.numEntries).vpn = vpn;
						TLB.table.at(TLB.repIndex % TLB.numEntries).pfn = PT.pageTable[vpn].pfn;
						if (verbose) {
							printf("  replace frame %d\n", frameRep);
							printf("  physical address is               0x%04x\n", physicalAddr);
							printf("  tlb update of vpn 0x%04x with pfn 0x%02x\n", vpn, PT.pageTable[vpn].pfn);
						}
					}
					numFault++;
				}
				TLB.repIndex++;
				numMiss++;
			}
			if (numAccess % upCount == 0) {
				for (int i = 0; i < coreMap.pageFrames; i++) {
					coreMap.map[i].use_vector = coreMap.map[i].use_vector >> 1;
				}
				if (verbose) {cout << "shift use vectors" << endl;}
			}
		}
	}
	cout << endl;

	cout << "statistics" << endl;
	cout << "  accesses    " << numAccess << endl;
	cout << "  tlb misses  " << numMiss << endl;
	cout << "  page faults " << numFault << endl << endl;

	if (verbose) {
		cout << "tlb" << endl;
		for (int i = 0; i < numEntries; i++) {
			printf("  valid = %d, vpn = 0x%04x, pfn = 0x%02x\n",
				TLB.table[i].valid,
				TLB.table[i].vpn,
				TLB.table[i].pfn);
		}
		cout << endl;

		cout << "core map table" << endl;
		for (int i = 0; i < pageFrames; i++) {
			printf(" pfn = 0x%02x: valid = %d, use vector = 0x%02x, vpn = 0x%04x\n",
				i,
				coreMap.map[i].valid,
				coreMap.map[i].use_vector,
				coreMap.map[i].vpn);
		}
		cout << endl;

		cout << "first ten entries of page table" << endl;
		for (int i = 0; i < 10; i++) {
			printf("vpn = 0x%04x: presence = %d, pfn = 0x%02x\n",
			i,
			PT.pageTable[i].presence,
			PT.pageTable[i].pfn);
		}
	}

	return 0;
}
