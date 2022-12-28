nsfaa (ver 1.0) by Yuri213212

使用协议：对于所有文件基本遵循CC BY-NC-SA 4.0，但是对于用户为了建立新工程而仿写的代码以及转换的数据无此限制。

本程序为aa文件编译器，可以编译生成FC模拟器用的NSF、NES、FDS文件以及纯ROM数据。

使用方法：
拖拽aa文件到nsfaa.exe程序图标上，程序将把编译结果输出到aa文件所在文件夹。
	aa文件的写法以及编译结果参见./sample里的示例工程。

语言设置：
修改language.txt可以设置程序的语言，程序支持英语、中文和日语，默认为系统语言，若系统语言不在支持语言列表里则默认英语。

文件格式：
aa	自创的语言“advanced assembler”语言源文件，程序代码可以做到与cm文件行单位的一一对应，使用nsfaa.exe编译，具体说明参见./doc/AdvancedAssembler.txt
	（建议设置文本编辑器高亮选项为HSP模式）
ah	aa版的头文件，用法和c语言的.h文件差不多，逻辑是在被include的位置展开，内容可以是纯定义，也可以带数据甚至作为partial class。
	（建议设置文本编辑器高亮选项为HSP模式）
cm	自创c风语言源文件（名字取自“c minus”，也可以理解为“comment”），数据类型只有长度区别并引入寄存器，用于程序设计（看成独立的注释文件），编译方法为手动编译，因此写法自由，只要你觉得很容易看懂
	对于c语言常见结构的手动编译方法参见./doc/C_Structure.txt
	（建议设置文本编辑器高亮选项为C/C++模式）

资料：
./doc/AdvancedAssembler.txt	aa语言的说明
./doc/ProgrammingGuide.txt	AdvancedAssembler编程指导，建议编程前先读该文档
./doc/6502*.txt			CPU指令集，数据分成4列：6502汇编命令与操作数，内存十六进制表现，执行周期，aa命令名
./doc/C_Structure.txt		c语言常见结构的手动编译方法
./doc/nsfMemory.txt		FC的内存结构以及NSF驱动的数据配置设计
./doc/SoundProtocol.txt		NSF驱动的协议列表，用于设计音乐数据编译器
./doc/tempo/tempo.txt		程序生成的文件，用于设置tempo
./doc/fmins/fmins.txt		VRC7常用FM音色列表
./doc/fxins.txt			N163常用波形音色列表
./doc/w64ins.txt		FDS常用波形音色列表
./doc/color.txt			调色板与常用颜色数据

示例工程：
./sample/nsf/00_empty		空NSF工程
./sample/nsf/01_soundtest	NSF驱动与芯片测试，第0曲为该芯片全通道合奏，之后的曲目为各通道独奏
./sample/nsf/02_wavtest		NSF形式的wav播放器，采样率22095.96Hz，由于NSF文件大小限制最多可以播放47秒
./sample/nes/00_empty		空NES工程
./sample/nes/01_helloworld	Hello world，没什么好解释的
./sample/nes/02_iotest		手柄与显示测试，按下手柄上的按键，屏幕上对应位置会显示一个方块
./sample/nes/03_spritetest	使用1P十字键控制游标（太阳符号）移动
./sample/nes/04_buttontest	按钮驱动测试，1P左右键按下时计数器±0x01，上下键抬起时计数器±0x10
./sample/nes/05_screentest	15Hz全屏刷新测试，1P十字键与A键按下时向4个区域写入大写字母，B键按下时改变颜色，建议将模拟器的连打设置为15Hz然后连打AB键看效果
./sample/nes/06_soundtest	使用N163扩展的NES播放器测试，1P十字键选曲，A键开始，B键停止
./sample/fds/*			这些工程的代码完全照搬对应的NES工程，用来测试将NES转换成FDS的可行性
./sample/bin/CP437		将CHR数据定义转换成ROM数据

参考文献：
6502 Reference				https://www.nesdev.org/obelisk-6502-guide/reference.html
NSF - Nesdev wiki			https://www.nesdev.org/wiki/NSF
INES - NESdev Wiki			https://www.nesdev.org/wiki/INES
FDS file format - NESdev Wiki		https://www.nesdev.org/wiki/FDS_file_format
FDS disk format - NESdev Wiki		https://www.nesdev.org/wiki/FDS_disk_format
VirtuaNES source code			http://virtuanes.s1.xrea.com/bin/virtuanessrc097.zip

更新说明：
旧版的NSF工程及其驱动已被废弃，相关数据决定移至将与本工程联动的工程fwg，敬请期待。
