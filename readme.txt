nsfaa (ver 0.2 alpha) by Yuri213212 (with tools and drivers to generate nsf music file)

使用协议：对于所有文件基本遵循CC BY-NC-SA 4.0，但是对于用户为了建立新nsf工程而仿写的代码以及转换的数据无此限制。

[程序构成]
./dmc2bin.exe	将dmc文件转换成数据模块的程序；推荐的使用方法是拖动dmc文件到程序上，或者使用bat处理
./nsfaa.exe	aa文件编译器；推荐的使用方法是拖动aa文件到程序上
./txt2bin.exe	将满足特定要求（详见各个驱动对应的模板）的it文件的文本转换成数据模块的程序（需要手动复制粘贴并添加配置文件）；推荐的使用方法是拖动配置文件到程序上

具体使用方法参考./music里的工程

[文件格式]
aa	自创的语言“advanced assembler”语言源文件，程序代码可以做到与cm文件行单位的一一对应，使用nsfaa.exe编译，具体说明参见./doc/AdvancedAssembler.txt
	（建议设置文本编辑器高亮选项为HSP模式）
bat	批处理文件，./music里的工程中使用的都是针对windows的，双击即可执行
bin	编译器以及其他工具输出的4KB对齐的数据模块，用来链接到nsf文件尾部，nsf程序中通过bankswitch调用链接的bin文件内容
cm	自创c风语言源文件（名字取自“c minus”），数据类型只有长度区别并引入寄存器，用于程序设计（看成独立的注释文件），编译方法为手动编译，因此写法自由，只要你觉得很容易看懂
	对于c语言常见结构的手动编译方法参见./doc/structure.txt
	（建议设置文本编辑器高亮选项为C/C++模式）
dmc	famitracker标准dpcm采样，为了链接需要将其扩展至4KB并添加长度等信息，使用dmc2bin.exe转换才能使用
it	modplug tracker编辑的音频工程，用于乐谱文件设计，可以半自动地转换成程序模块，具体参见./music/readme.txt
nsf	FC音乐程序模块，通过nsfaa.exe编译得到，有时编译的结果只是最开头的模块（参见./music里的工程），需要链接bin文件才能正常使用
txt	音乐工程中是把it文件转换成bin的中间格式，每个文件是一页it文件文本，具体参见./music/readme.txt

[资料]
./doc/6502*.txt			CPU指令集，数据分成4列：6502汇编命令与操作数，内存十六进制表现，执行周期，aa命令名
./doc/AdvancedAssembler.txt	aa语言的说明
./doc/fxins.txt			N163常用波形音色列表
./doc/memory.txt		FC的内存结构以及驱动的数据配置设计
./doc/ProgrammingGuide.txt	AdvancedAssembler编程指导，建议编程前先读该文档
./doc/reset.txt			从virtuanes源码总结出来的音源相位重启时机
./doc/structure.txt		c语言常见结构的手动编译方法
./doc/template.txt		aa文件的模板，写新nsf程序的时候建议复制该文件改造，或者从该文件复制所有需要的部分
./doc/w64ins.txt		FDS常用波形音色列表
./doc/fmins/fmins.txt		VRC7常用FM音色列表
./doc/tuning/f.xls		根据virtuanes源码，对音源频率精确计算的表格，用来调音
./drivers/annotation.txt	it文件的注解说明
./drivers/itdata.txt		模板it文件的通道说明
./drivers/readme.txt		模板说明
./drivers/configguide.txt	txt2bin配置文件说明
./drivers/itdata.txt		it文件的数据说明
./drivers/readme.txt		nsf驱动说明
./music/readme.txt		音乐工程的构造与编译说明

[音色库]
./libdmc	nsf文件用dpcm音色库
./libit		it文件用音色库

[aa工程]
./tests		音源测试用程序，如果想从0开始写程序建议先看懂这部分，学会每个APU的使用方法
./drivers	我的nsf驱动模块，分为6大类，如果只想直接写乐谱数据的话可以考虑以此为模板改造
./music		使用drivers下程序模块的音乐工程

[外部工具与主要使用方式]
VirtuaNES	（nsf执行环境）		http://virtuanes.s1.xrea.com/vnes_dl.php
ModPlug Tracker	（主数据编辑器）	https://download.openmpt.org/archive/mpt_classic/
OpenMPT		（辅助数据编辑器）	http://openmpt.org/download
FamiTracker	（图形DPCM转换器）	http://famitracker.com/downloads.php
Virtual ear	（虚拟示波器）		https://github.com/Yuri213212/vear

[参考文献]
VirtuaNES source code			http://virtuanes.s1.xrea.com/bin/virtuanessrc097.zip
NSF - Nesdev wiki			https://wiki.nesdev.com/w/index.php/NSF
6502 Reference				http://obelisk.me.uk/6502/reference.html
Programming with unofficial opcodes	https://wiki.nesdev.com/w/index.php/Programming_with_unofficial_opcodes

[TODO]
完成fwg工程并与本工程联动，消除对外部编辑器的依赖并提供更加自动化的音乐工程编译环境
