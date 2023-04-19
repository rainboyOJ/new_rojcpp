## 说明与目标

本程序是[QingdaoU/Judger](https://github.com/QingdaoU/Judger)的cpp复刻版，目标：

- 使用c++17标准重新编写，目的是学习OJ judger的原理
- 可以给oier，acmer在本地使用

## 文件

```plaintext
judger.h    重用的定义
```

## 安装

```bash
git clone https://github.com/rainboyOJ/judgercpp
cd judgercpp
make
sudo make install
```

### 在服务器上安装

```bash
git clone https://github.com/rainboyOJ/judgercpp
cd judgercpp
make local=no
sudo make install
```

## 使用

## Judger Server

请看这里[ TODO ](#)

```
.
├── commandline.hpp     命令行库
├── judge_core.cpp      核心代码
├── judger.h            相关定义
├── judge_wrapper.cpp   封装,可以给用户在命令行上调用,使用更少的运行参数
```
