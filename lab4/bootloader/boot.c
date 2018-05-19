#include "boot.h"
#include<string.h>


#define SECTSIZE 512

typedef unsigned int   uint32_t;
typedef unsigned char  uint8_t;

void bootMain(void) {
	/* 加载内核至内存，并跳转执行 */
	struct ELFHeader* elf;
	struct ProgramHeader* ph;
	struct ProgramHeader* eph;
	uint8_t* buf= (uint8_t* )0x400000;
	//读取磁盘中的到buf中	
	for(int i=1;i<201;i++)		
		readSect((void*)(buf+(i-1)*512),i);
	//elf=(void*)buf;
	elf=(struct ELFHeader*)buf;
	//开始分析elf头得到程序地址
	ph = (void*)elf + elf->phoff;
	//ph = (struct ProgramHeader *)(buf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph ;ph ++)
	{
		if (1 == ph->type)
		{
			uint32_t vaddr = ph->vaddr;
			memcpy((void*)vaddr , (void*)(buf + ph->off) , ph->filesz);
			//清空多余的位置			
			if(ph->memsz - ph->filesz > 0)
				memset((void*)vaddr + ph->filesz,0,ph->memsz-ph->filesz);

		} 
		//ph++;
	}
	//跳转到kernel
	void (*entry)(void);
	entry = (void*)(elf->entry);
	entry();

}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}
