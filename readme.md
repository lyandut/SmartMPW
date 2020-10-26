# SmartMPW

---

2020（第二届）集成电路EDA设计精英挑战赛

赛题三：**智能 MPW 拼接**

## 编译

- Win + VS2017
  - open `SmartMPW.sln`
- Win + MinGW
  - `g++ SmartMPW/*.cpp SmartMPW/*.hpp -o placement.exe -I %BOOST_ROOT%`
  - `g++ SmartMPW/*.cpp SmartMPW/*.hpp -o placement.exe -I %BOOST_ROOT% –D __USE_XOPEN2K8 -s -O2`

## 运行

- `placement.exe /xxx/xxx/input.txt`
- `placement.exe all`
- `placement input`