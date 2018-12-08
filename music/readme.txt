工程的结构与编译方法：
./data	乐谱数据
		找出最能满足要求的驱动类别
		手动修改it文件以满足该驱动类别的模板格式
			使用libit文件夹下的音色库定义波形
			请不要随意修改模板通道的增益，当前的增益值基本反映了模拟器的输出增益，并且修改不会在最终输出中反映
			请不要更改默认乐器的顺序，数据转换器依赖默认的乐器顺序
			模板的第一页描述了所有有效数据或数据有效范围，请不要超出限制
			请注意通道乐器的转调、波形的速度（是否低八度）以及波形的范围（单电源还是双电源，响度差6dB）
		使用modplug tracker或openmpt从it文件复制数据到##.txt文件（对所有页码重复以下流程：页码上右键copy pattern，新建＜页码＞.txt文件，打开粘贴保存关闭）
		添加config.txt文件（第一行为模板的通道数，第二行为尾页的页码，详见configguide.txt）
		拖拽config.txt文件到txt2bin.exe执行
		写链接所有bin文件的bat文件并执行（link.bat）
		复制得到的bin文件（data.bin）到上一层文件夹
./dmc	dpcm采样
		从libdmc文件夹复制需要的dmc文件
		写对于所有dmc文件执行dmc2bin.exe的bat文件并执行（gen.bat）
		写链接所有bin文件的bat文件并执行（link.bat）
		复制得到的bin文件（dmc.bin）到上一层文件夹
./
		复制driver文件夹下指定驱动类别的config##.aa、program##.bin和dummysub##.bin（如果存在）
		改造config##.aa并编译
		写链接config##.nsf与所有bin文件的bat文件并执行（link.bat）
			得到最终的nsf文件

特殊工程（模板分声道测试工程）：
./tests/test##	对于每个模板进行所有声道的单声道测试，通过测试意味着数据变换和执行基本正常
