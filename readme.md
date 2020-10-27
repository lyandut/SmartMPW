# SmartMPW

---

2020（第二届）集成电路EDA设计精英挑战赛

赛题三：**智能 MPW 拼接**

## 编译

- Win + VS2017
  - just open `SmartMPW.sln`.
- Win + MinGW
  - `g++ SmartMPW/*.cpp SmartMPW/*.hpp -o placement.exe -I %BOOST_ROOT% -s -O2`
- Linux + GCC
  - `source /home/mpw/makesetup.csh`
  - `g++ –D__USE_XOPEN2K8 -O2 SmartMPW/*.cpp SmartMPW/*.hpp -o placement.exe -I/path/to/boost_include_dirs -std=c++11`

## 运行

- `placement.exe /xxx/xxx/input.txt`
- `placement.exe all`
- `placement input`

g++ -Iboost_1_73_0 -Wl,-rpath=/home/env/gcc482/linux26-x86_64/lib -static-libstdc++ -std=c++11 -D__USE_XOPEN2K8 -DBOOST_GEOMETRY_DISABLE_DEPRECATED_03_WARNING SmartMPW/*.cpp SmartMPW/*.hpp -o Deploy/placement.exe