## MentoHUST与Mento Supplicant和mystar的关系？ ##
  * 最初的MentoHUST是在Mento Supplicant V3.8版的基础上修改的，在第二个版本中加入了DHCP功能以及Soar的客户端校验并简化了Alog、Blog两个算法，再后来对于Soar的客户端校验算法做了改进，对于认证流程也做了修改，MentoHUST与原有Mento的差异越来越大。总的来说，可以把MentoHUST看做对Mento的继承与发展。
  * 由于在编写MentoHUST for Linux之前，本人无任何gcc经验，所以在编写Linux版时参考了mystar的Makefile等文档。在代码上，MentoHUST for Linux主要是以Windows版的MentoHUST为参考，对于mystar并无多少借鉴。

## MentoHUST是否完全兼容锐捷？ ##
  * 在成员kkHAIKE的努力下，锐捷V3客户端校验算法已被解决，Linux版MentoHUST已基本完全兼容锐捷目前所有版本。
  * 由于官方Windows版锐捷也在完善，所以基本不会对Windows版MentoHUST加入V3算法支持。

## MentoHUST for Linux如何使用？ ##
  * 请参考[锐捷、赛尔认证MentoHUST](http://wiki.ubuntu.org.cn/%E9%94%90%E6%8D%B7%E3%80%81%E8%B5%9B%E5%B0%94%E8%AE%A4%E8%AF%81MentoHUST)

## MentoHUST使用起来比其他客户端麻烦？ ##
  * 这是对于MentoHUST的误解。MentoHUST的默认发包是与xrgsu兼容的，一般没有开启客户端校验的学校都开放了xrgsu的认证，也就是说学校没有开启客户端校验的情况下，一般是可以无需配置mpf文件即可通过认证的，这时使用MentoHUST绝对不比官方的xrgsu麻烦。
  * MentoHUST的“自定义数据文件”只是一个可选功能，用于不使用数据文件认证失败后，这种情况在0.3.3版以后很少发生。
  * 有些学校虽然没开启客户端校验，但对于版本有特定要求，现有其他第三方锐捷客户端要兼容一个新版本，必须通过修改源代码来实现，这对于对代码不太熟悉的人来说并不是件轻松的事情，而使用MentoHUST的“自定义数据文件”功能则可无需修改任何代码就实现这点。
  * 总之，MentoHUST的“自定义数据文件”只是一个可选功能，其他客户端能认证的情况下MentoHUST基本上不使用这一功能也可认证，使得操作变麻烦的“自定义数据文件”功能是为了更好兼容。

## 我想装Linux，但又担心在Linux下不能通过锐捷认证，能否提前知道这点？ ##
  * Linux版MentoHUST已完全兼容目前所有版本锐捷，所以请放心大胆的投入Linux的怀抱中来吧！

## MentoHUST能否移植到其他平台？ ##
  * MentoHUST V0.2.5版及0.3.0版已成功在MacOS上编译，运行良好([下载](http://mentohust.googlecode.com/issues/attachment?aid=2121908789865846359&name=mentohust_mac.tar.gz))。

## MentoHUST能否支持其他客户端的认证？ ##
  * MentoHUST还支持赛尔客户端的认证。

## 提示参考 ##
  * 提示“在网卡eth0上获取IP失败”：如果是动态IP的话不用理会；静态IP的话，请到NetworkManager或其他网络设置工具中设置IP
  * 提示“IP地址类型错误”：DHCP方式选错了。使用-q（mentohust -h查看其帮助）参数查看正确的DHCP方式并按需修改（锐捷中的“认证前”可能需要用mentohust中的“二次认证”代替）
  * 提示“IP端口绑定错误”：这出现在静态IP的时候，原因是在MentoHUST中将绑定IP设置错误，可以通过-i参数修改。
  * 提示“找不到服务器”：一般是选错了组播模式，在“标准”与“锐捷”中切换试试。
  * 认证成功但无法上网：静态IP用户未正确设置IP及DNS或动态IP未正确获取到IP及DNS，前者通过网络设置工具（例如NetworkManager）修改，后者请正确设置DHCP方式（在没有dhclient的系统中还需找个有效替代并使用-c参数修改）
  * 提示“不允许使用的客户端类型”：学校禁用了xrgsu，使用-v参数指定版本号，或者复制相关文件（"8021x.exe"和"W32N55.dll"，可能还需要"SuConfig.dat"）到/etc/mentohust/
  * 提示“客户端版本过低”：同上
  * 提示“客户端完整性被破坏”：学校开启了客户端校验，复制相关文件（"8021x.exe"和"W32N55.dll"，可能还需要"SuConfig.dat"）到/etc/mentohust/

_OMG，没想到我居然写了这么多，就这样吧。欢迎高手参与进来共同开发维护MentoHUST！2009.9.25 修改于2009.12.6 修改于2010.5.12_