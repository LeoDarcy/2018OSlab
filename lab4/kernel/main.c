#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {
	//assert(0);
	initSerial();// initialize serial port finish
	initIdt(); // initialize idt finish
	initIntr(); // iniialize 8259a finish
	//assert(0);
	initTimer();
	initSeg(); // initialize gdt, tss
	initPcb();//初始化pcb	
	initSema();//初始化sema信号量
	//assert(0);
	loadUMain(); // load user program, enter user space

	while(1);
	assert(0);
}
