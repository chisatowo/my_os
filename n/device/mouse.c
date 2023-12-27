#include "mouse.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

#define MOS_BUF_PORT 0x60 // 键盘buffer寄存器端口号为0x60
#define MOS_DATA_PORT 0x64 // 鼠标buffer寄存器端口号为0x64
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

static int8_t set;
static int16_t buffer[3];
static int8_t button;
static int16_t x, y;

static int16_t old_buffer;
static int16_t old_x;
static int16_t old_y;
static int16_t new_x;
static int16_t new_y;
static int16_t x_location;
static int16_t y_location;
static int8_t mouse_status;

void waiMSEReady()
{
    for (;;)
    {
        if ((inb(MOS_BUF_PORT) & 0x02) == 0)
        {
            break;
        }
    }
}

/* 鼠标中断处理程序 */
static void intr_mouse_handler(void)
{
    uint16_t a = inb(MOS_BUF_PORT);
    // 接收第一次收到的0xfa,并标志鼠标已启动
    if (mouse_status == 0 && a == 0xfa)
    {
        put_str("READY ");
        mouse_status = 1;
        return;
    }
    
    //等待鼠标传输信息，一共需要传输三个字节的信息
    buffer[set] = a;

    set = (set + 1) % 3;
    // 已连续录入三个字节的信息,现在开始解析
    // 恢复鼠标上一个光标位置的属性，结合mouse_move实现跟随光标
    // 由于我们在后面打印了字符串，并让窗口上卷，所以这里要检测一下
    if((old_x * 2 + (old_y-1) * 160) > 0)
    	mouse_recover(old_x * 2 + (old_y-1) * 160);
    if (set == 0)
    {
        // 记录按键
        uint8_t i = 0;
        for (; i < 3; i++)
        {
            if (buffer[0] & (0x1 << i))
            {
                button = i;
		if(button==0)
                	put_str("Click left_btn\n");
		if(button==1)
			put_str("Click right_btn\n");
		if(button==2)
			put_str("Click mid_btn\n");
		button=-1;
		return;
            }
        }

        // 判断x是否为负
        if (buffer[0] & 0x10)
        {
	//0xff就是-1，遵循对称性原则[0x4C,0xFF],[0x00,0xB3],[-180,-1],[0,179]
            new_x = x - (0xff - buffer[1])-1;
        }
        else
        {
            new_x = x + buffer[1];
        }
        // 判断y是否为负,调整鼠标的坐标系，使整个屏幕处在第一象限，坐标原点在屏幕左上角
	// x向右为+，y向下为+
        if (buffer[0] & 0x20)
        {
	//这里与x的处理相反，因为我们向
            new_y = y + (0xff - buffer[2])+1;
        }
        else
        {
            new_y = y - buffer[2];
        }
	/*输出原来的信息，用于调试
	put_int(buffer[1]);
	put_str("\n");
	put_int(buffer[2]);
	put_str("\n");
	put_str("*************\n");*/
        x_location = new_x / 4;
        y_location = new_y / 8;
//这一段的作用是不让光标位置超出屏幕
        if (x_location >= 80)
            x_location = 79;
        if (x_location < 0)
            x_location = 0;
        if (y_location >= 25)
            y_location = 24;
        if (y_location < 0)
            y_location = 0;
	mouse_move(x_location * 2 + y_location * 160);
	//输出将坐标原点为屏幕左上角转换后x和y的坐标位置
	put_str("x: ");
	put_int(new_x);
	put_str("      ");
	put_str("y: ");
	put_int(new_y);
	put_str("\n");
        old_x = x_location;
        old_y = y_location;
        old_buffer = buffer[0];
    }
    return;
}
/* 鼠标初始化 */

void mouse_init()
{   
    mouse_status = 0;
    set = 0;
    old_x = 0;
    old_y = 0;
    new_x = 0;
    new_y = 0;
    button=-1;
    x_location = 0;
    y_location = 0;
    put_str("mouse init start\n");
    //waiMSEReady();
    //向控制寄存器0x64输入0x60进入数据接受状态
    outb(MOS_DATA_PORT, MOS_BUF_PORT);
    //waiMSEReady();
    // 向输入缓冲区输入数据0x47启动鼠标模式，通过键盘电路端口0x60读取鼠标信息
    outb(MOS_BUF_PORT, 0x47);
    //waiMSEReady();
    outb(MOS_DATA_PORT, 0xd4);
    //waiMSEReady();
    // 0xf4数据激活鼠标电路，激活后将会给CPU发送中断信号
    outb(MOS_BUF_PORT, 0xf4);
    //注册中断处理函数
    register_handler(0x2c, intr_mouse_handler);
    //向端口发送初始化信息
    outb(MOS_DATA_PORT, 0x66);
    //设置初始光标位置到屏幕右上角
    //因为坐标范围为x:[-180,180);y[-124,100]
    x = 180; // [0x00,0xb3]
    y = 101; // [0x00,0x64]
    old_buffer = 0;
    put_str("mouse init done\n");
}
