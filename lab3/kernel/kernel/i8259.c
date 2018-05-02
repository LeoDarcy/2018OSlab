#include "x86.h"

#define PORT_PIC_MASTER 0x20
#define PORT_PIC_SLAVE  0xA0
#define IRQ_SLAVE       2

/* 初始化8259中断控制器：
 * 硬件中断IRQ从32号开始，自动发送EOI */
void
initIntr(void) {
	outByte(PORT_PIC_MASTER + 1, 0xFF);
	outByte(PORT_PIC_SLAVE + 1 , 0xFF);
	outByte(PORT_PIC_MASTER, 0x11); // Initialization command
	outByte(PORT_PIC_MASTER + 1, 32); // Interrupt Vector Offset 0x20
	outByte(PORT_PIC_MASTER + 1, 1 << 2); // Tell Master PIC that there is a slave
	outByte(PORT_PIC_MASTER + 1, 0x3); // Auto EOI in 8086/88 mode
	outByte(PORT_PIC_SLAVE, 0x11); // Initialization command
	outByte(PORT_PIC_SLAVE + 1, 32 + 8); // Interrupt Vector Offset 0x28
	outByte(PORT_PIC_SLAVE + 1, 2); // Tell Slave PIC its cascade identity
	outByte(PORT_PIC_SLAVE + 1, 0x3); // Auto EOI in 8086/88 mode

	outByte(PORT_PIC_MASTER, 0x68);
	outByte(PORT_PIC_MASTER, 0x0A);
	outByte(PORT_PIC_SLAVE, 0x68);
	outByte(PORT_PIC_SLAVE, 0x0A);
}

/*
lab3用于使用以下代码对8253可编程计时器进行设置，使得8253以频率HZ产生时间中断信号发送给8259A可编程中断控制器；若依照代码框架lab2/kernel/kernel/i8259.c中给出的配置示例对8259A进行设置，则该中断的中断向量为0x20，对此中断设置时间中断处理程序
*/

#define TIMER_PORT 0x40
#define FREQ_8253 1193182
#define HZ 100

void initTimer() {
    int counter = FREQ_8253 / HZ;
    outByte(TIMER_PORT + 3, 0x34);
    outByte(TIMER_PORT + 0, counter % 256);
    outByte(TIMER_PORT + 0, counter / 256);
}
