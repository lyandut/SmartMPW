# SmartMPW

---

2020（第二届）集成电路EDA设计精英挑战赛

赛题三：**智能 MPW 拼接**

## 编译

- Win + VS2017
  - just open `SmartMPW.sln`.
- Win + MinGW64 8.1.0
  - `g++ *.hpp *.cpp -O2 -std=c++11 -o placement.exe`
- Linux + GCC 4.8.2（官方提供的服务器环境，提交版本）
  - `csh`
  - `source /home/mpw/makesetup.csh`
  - `g++ *.hpp *.cpp -D__USE_XOPEN2K8 -DSUBMIT -DNDEBUG -O2 -std=c++11 -o placement.exe`
    - `-static-libstdc++` 静态链接libstdc++防止动态库版本不匹配，出现 `GLIBCXX_3.4.18` not found 错误时需要指定
    - `-D__USE_XOPEN2K8` 官方《编译指南》要求（必须）
    - `-DSUBMIT` 自定义宏，用于区分测试/提交版本（必须）
    - `-DNDEBUG` release版本关闭assert断言（必须）
    - `-O2` 编译优化（可选）
    - `-std=c++11` 指定c++标准（必须）

## 运行

- `placement.exe /xxx/xxx/input_<id>.txt`（开启`-DSUBMIT`）
- `placement.exe --all`（关闭`-DSUBMIT`关闭，`Instance/`，`Solution/`，`placement.exe`位于同一目录下）
