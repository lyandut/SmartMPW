# SmartMPW

---

2020（第二届）集成电路EDA设计精英挑战赛

赛题三：**智能 MPW 拼接**

## 编译

- Win + VS2017
  - just open `SmartMPW.sln`.
- Win + MinGW64 8.1.0
  - `g++ SmartMPW/*.hpp SmartMPW/*.cpp -I%BOOST_ROOT% -O2 -std=c++11 -o placement.exe`
- Linux + GCC 4.8.2（官方提供的服务器环境）
  - `csh`
  - `source /home/mpw/makesetup.csh`
  - `g++ SmartMPW/*.hpp SmartMPW/*.cpp -Iboost_1_73_0 -static-libstdc++ -D__USE_XOPEN2K8 -DBOOST_GEOMETRY_DISABLE_DEPRECATED_03_WARNING -DSUBMIT -DNDEBUG -O2 -std=c++11 -o placement.exe`
    - `-Iboost_1_73_0` 依赖boost_geometry库（必须）
    - `-static-libstdc++` 静态链接libstdc++防止动态库版本不匹配（必须）
    - `-D__USE_XOPEN2K8` 官方《编译指南》要求（必须）
    - `-DBOOST_GEOMETRY_DISABLE_DEPRECATED_03_WARNING` 关闭boost相关警告（可选）
    - `-DSUBMIT` 自定义宏，用于区分测试/提交版本（必须）
    - `-DNDEBUG` release版本关闭assert断言和重叠检测（必须）
    - `-O2` 编译优化（可选）
    - `-std=c++11` 指定c++标准（必须）

## 运行

- `placement.exe /xxx/xxx/input.txt`（要求`-DSUBMIT`开启）
- `placement.exe --all`（要求`-DSUBMIT`关闭，`Instance/`，`Solution/`，`placement.exe`位于同一目录下）
