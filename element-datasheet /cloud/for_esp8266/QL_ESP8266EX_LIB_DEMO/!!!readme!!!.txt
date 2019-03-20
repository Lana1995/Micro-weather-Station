1、ESP8266开发环境搭建及编译生成固件的步骤，请参考乐鑫官网http://espressif.com/zh-hans/support/download/documents提供的文档，目前我们支持1MFlash、4MFlash的8266模组。

2、请将demo文件夹拷贝至SDK根目录，将libqinglianyun.a文件拷贝至SDK根目录下的lib文件夹。

3、执行gen_misc.sh生成bin文件。

4、本案例以集成了4M flash的ESP8266模块为硬件平台，如需换成1Mflash，请更改头文件qlcloud_interface.h中#define EXT_FLASH_SIZE的定义，并按生成1M固件的流程更改gen_misc.sh文件。