<font size=5>**nnie-yolov3**</font>

**platform:** 

<1>  VS2017 (v141工具集，debug x64) solution   或者

<2>  HISI3516CV500(Makefile)

**Inspiration:**

海思的文档以及ruyistudio的环境，调试、开发很是不方便, 因此简单重新组织了下NNIE的代码，把yolov3从中抽取了出来，方便调试，并提供了vs工程，cuda加速 

**Dependency:**

opencv: 解压opencv_world341d.rar 到.lsn路径下， 其他芯片需要自己编译opencv替换opencv/hisi3516CV500

**Instruction:**

可以使用VS2017 v141/v140工具集 debug x64生成解决方案，本项目提供了vs2017和HISI3516CV500 Makefile

main.cpp 1-2行 

```
#define FUNC 
#define INST 
```

用来控制func模拟还是inst模拟

**Compile:**

make生成sample_yolov3_hisi3516CV500.bin, 需要注意的是

<1> **Makefile中的路径需要改成你自己的路径**

<2> **链接的时候最好链接静态库，不要链接动态库**

