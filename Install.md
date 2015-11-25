本文介绍两种安装MentoHUST的方法：
  * 依据平台来安装MentoHUST：
  * 手动编译安装MentoHUST：
另外，也提供一些设置开启启动的方法。

# 二进制软件包 #
## Fedora发行版 ##
请[下载最新的RPM软件包](http://code.google.com/p/mentohust/downloads/list?q=rpm)，然后切换至对应目录，在命令行中输入以下命令：
```
rpm -ivh "rpm包文件名"
```

## Ubuntu发行版 ##
请[下载最新的DEB软件包](http://code.google.com/p/mentohust/downloads/list?q=deb)，双击deb包即可安装。
如果无法双击安装，可使用命令行操作：
```
sudo dpkg -i "deb包文件名"
```

## `ArchLinux`发行版 ##
目前，`ArchLinux`发行版的用户可以在AUR中搜索mentohust软件包来进行安装。或者点击以下下链接直接查
看：
[AUR上mentohust包](http://aur.archlinux.org/packages.php?ID=33531)

对于安装了 yaourt 软件的用户，可以直接运行以下命令安装：
```
yaourt -S mentohust
```
另外，你也可以直接下载已经编译好的软件包（[x86\_64平台](http://mentohust.googlecode.com/files/mentohust-0.3.1-2-x86_64.pkg.tar.gz)或[i686平台](http://mentohust.googlecode.com/files/mentohust-0.3.1-2-i686.pkg.tar.gz)  )。

## 手工安装 ##
如果所用系统并没有对应的软件包，可使用解压软件从deb包提取文件或者自己编译（MacOS就别从deb包提取了，而应该下载对应的MacOS版），然后复制文件到/usr/bin下：
```
sudo cp ./mentohust /usr/bin/mentohust
sudo +x /usr/bin/mentohust
```
不会敲命令的可输入以下命令后像Windows下一样操作：
```
sudo xdg-open /
```

# 手动编译 #
> ## 准备好源代码 ##
> 您可以直接[下载源代码包](http://code.google.com/p/mentohust/downloads/list?q=tar.gz)或者运行以下命令从svn仓库取出最新版代码并生成对应的configure文件（需要安装相应工具链）：
```
svn checkout http://mentohust.googlecode.com/svn/trunk/ mentohust
cd mentohust
sh autogen.sh
```

> ## 配置MentoHUST ##
> 目前您可以根据自己的需求启动或禁止以下特性：
```
--enable-debug         Build a debug version
--disable-encodepass   Don't encode password
--disable-arp          Don't care ARP info
--disable-notify       Don't show notification
--disable-nls          Disable NLS
--with-pcap=TYPE       How to link libpcap: dyload(Default) | dylib | stlib | PATH
```
> 对于普通PC用户，一般运行这个命令配置即可：
```
./configure --prefix=/usr
```
> 对于交叉编译的用户，建议禁用所有特性并链接libpcap而不是动态载入：
```
./configure --host=mipsel-linux --disable-encodepass --disable-arp --disable-notify --disable-nls --with-pcap=dylib
```

> ## 编译并安装 ##
```
make && make install
```

> ## 给程序授予超级用户权限（可选） ##
```
sudo chmod u+s /usr/bin/mentohust
```
如果您不运行这个命令，那么以后启动mentohust，均需要注意以root权限运行（例如，`sudo mentohust`）

# 开机运行 #
## Ubuntu、Fedora用户 ##
选择菜单“系统->首选项->启动应用程序”，点击“添加”，输入名称MentoHUST，命令sudo mentohust，点击“添加”即可。

对于没有修改/etc/sudoers文件使得sudo无需密码的用户，此方法可能无效，这时可通过`sudo chmod u+s /usr/bin/mentohust`授予权限然后按此方法添加命令为mentohust的启动项。

## `ArchLinux`用户 ##
AUR中的mentoHUST软件包提供了一个rc脚本，因此直接在系统配置文件/etc/rc.conf中的DAEMONS节添加'mentohust' 即可：
```
DAEMONS=(syslog-ng network ... @mentohust ...)
```

## rc.local方式 ##
修改**/etc/rc.local\*或者**/etc/gdm/Init/Default\*等文件，例如：
```
sudo gedit /etc/gdm/Init/Default
```
然后在exit 0（最后一行）前面加入以下内容：
```
if [ -x /usr/bin/mentohust ]; then
    /usr/bin/mentohust
fi
```

## MacOS用户 ##
待编辑。

## 小技巧 ##
  * 建议需要开机运行的用户使用以下命令将MentoHUST设置为daemon运行并保存输出到**`/tmp/mentohust.log`**，开启消息通知。
```
sudo mentohust -b3 -y5 -w
```