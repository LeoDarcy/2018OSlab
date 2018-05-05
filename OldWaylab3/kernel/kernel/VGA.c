#include "x86.h"
#include "device.h"

//使用游标解决输出问题，可以避免使用全局变量导致变量竞争
//参考网站https://blog.csdn.net/gemini_star/article/details/4438280


//VGA显卡内部有一系列寄存器可以用来控制显卡的状态。在标准的PC机上。 0x3d4和0x3d5两个端口可以用来读写显卡的内部寄存器。方法是先向0x3d4端口写入要访问的寄存器编号，再通过0x3d5端口来读写寄存器数据。存放光标位置的寄存器编号为14和15。两个寄存器合起来组成一个16位整数，这个整数就是光标的位置。比如0表示光标在第0行第0列，81表示第1行第1列（屏幕总共80列）
void SetCursor(int row,int col)
{
	unsigned short pos = row * 80 + col;  
	outByte(0x3d4, 14);  
        //高位放14号寄存器  
	outByte(0x3d5, (pos >> 8) & 0xff);  
	outByte(0x3d4, 15);  
        //低位放15号寄存器  
	outByte(0x3d5, pos & 0xff); 
}
