# SmartMPW

---

2020（第二届）集成电路EDA设计精英挑战赛

赛题三：**智能 MPW 拼接**

## 编译

- Win + VS2017
  - just open `SmartMPW.sln`.
- Win + MinGW64 8.1.0
  - `g++ SmartMPW/*.hpp SmartMPW/*.cpp -O2 -std=c++11 -o placement.exe`
- Linux + GCC 4.8.2（官方提供的服务器环境）
  - `csh`
  - `source /home/mpw/makesetup.csh`
  - `g++ SmartMPW/*.hpp SmartMPW/*.cpp -static-libstdc++ -D__USE_XOPEN2K8 -DSUBMIT -DNDEBUG -O2 -std=c++11 -o placement.exe`
    - `-static-libstdc++` 静态链接libstdc++防止动态库版本不匹配（必须）
    - `-D__USE_XOPEN2K8` 官方《编译指南》要求（必须）
    - `-DSUBMIT` 自定义宏，用于区分测试/提交版本（必须）
    - `-DNDEBUG` release版本关闭assert断言（可选）
    - `-O2` 编译优化（可选）
    - `-std=c++11` 指定c++标准（必须）

## 运行

- `placement.exe /xxx/xxx/input.txt`（要求`-DSUBMIT`开启）
- `placement.exe --all`（要求`-DSUBMIT`关闭，`Instance/`，`Solution/`，`placement.exe`位于同一目录下）
