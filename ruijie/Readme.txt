#使用方法（需要管理员权限），本使用方法修改自mystar，注意文件名区分大小写

#修改数据包（可选，适用于有锐捷客户端版本要求的同学）
下载“MentoHUSTTool“(MentoHUST数据包修改工具)和相应锐捷版本的MPF文件，按MentoHUSTTool中步骤操作。也可按下面方法手工修改：
MPF文件的格式是前部分根据锐捷8021x.exe文件生成，后部分填充相同大小（不小于0x80字节）的锐捷认证时的Start包和Md5包，用WinHex打开MPF文件并“查找十六进制数值”0000131138303231782E657865，即可找出二者界限
使用WireShark抓取Windows下锐捷正常认证时的数据并保存为“锐捷.pcap”
到http://lewuya.ys168.com/或者http://pcyard.qupan.com/下载相应版本的MPF文件“版本.mpf”
用WinHex打开“锐捷.pcap”和“版本.mpf”，在“版本.mpf”中“查找十六进制数值”0000131138303231782E657865并删除从此处往后的数据
在“锐捷.pcap”中找到Start包部分并复制粘贴“地址校验值”以后的数据（从0000131138303231782E657865开始）到“版本.mpf”最后面，找到Md5包部分并复制粘贴相同大小相应数据到“版本.mpf”最后面
保存“版本.mpf”文件

#设置数据包（可选，适用于有锐捷客户端版本要求的同学）
假设数据包文件名为“版本.mpf”
sudo mkdir /etc/mentohust
sudo cp 版本.mpf /etc/mentohust
修改mentohust.conf使Package=/etc/mentohust/版本.mpf

#将mentohust.conf修改好后放在/etc/mentohust(具体修改方法见mentohust.conf)
sudo cp mentohust.conf /etc/mentohust

#设置可执行权限
sudo chmod +x MentoHUST
#将程序放到/bin（可选，使得运行MentoHUST不需要带路径）
sudo cp MentoHUST /bin
#设置超级用户权限（可选，使普通用户正常执行MentoHUST）
sudo chmod +s /bin/MentoHUST

#开机自动认证（方法有多种）
#使用/etc/rc.local脚本实现
将以下代码添加至/etc/rc.local文件的末尾：MentoHUST &
#在桌面环境程序中添加自动运行项来实现
KDE4桌面可在“系统设置”->“高级”->"自动启动"中添加程序。填入/bin/MentoHUST即可
Gnome桌面可以在“系统”->“会话“->“自动启动”中添加程序。

#后台运行
在命令行结尾加&后运行程序，可关闭终端，程序在后台继续运行。
若想将输出结果重定向到文件：
MentoHUST >file 把stdout重定向到file文件中
MentoHUST 1>file1 把stdout重定向到file1

#依赖项(一般只要有/usr/lib/libpcap.so就可以编译)
#32位环境下编译时依赖于
linux-gate.so.1
libpcap.so.0.9 => /usr/lib/libpcap.so.0.9 #使用libpcap动态库编译时依赖此项
libc.so.6 => /lib/libc.so.6
/lib/ld-linux.so.2
#64位环境下编译时依赖于
linux-vdso.so.1
libpcap.so.0.8 => /usr/lib/libpcap.so.0.8
libc.so.6 => /lib/libc.so.6
/lib64/ld-linux-x86-64.so.2

#权责声明
1、本程序所有涉及锐捷认证的功能均是来自前辈公开代码及抓包分析。
2、本程序于个人仅供学习，于他人仅供方便认证，不得使用本程序有意妨害锐捷认证机制及相关方利益。
3、本程序不可用于任何商业和不良用途，否则一切后果自负。
4、本程序不提供任何服务及保障，编写及维护纯属个人爱好，随时可能被终止。
5、使用本程序者，即表示同意该声明。谢谢合作。
