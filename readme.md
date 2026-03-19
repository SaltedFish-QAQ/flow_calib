# ReadMe
## 依赖包与编译环境

- **1、编译环境**
    本工程使用的编译环境以及设置如下
    | 名称 | 版本 |
    |------|------|
    | keil | 5.43.0.0 |
    | c complier | 6.24 |
    | assembler | 6.24 |
    | linker/locator | 6.24 |
    | library manager | 6.24 |
    | hex converter | 6.24 |
    
    - **Note**：**请使用keil 5版本编译此工程并且选择编译器为acc6而非acc5**
- **2、依赖包**
    本工程依赖rtthread nano版本，github链接如下
    https://github.com/RT-Thread/rtthread-nano
    keil下移植与安装教程如下
    https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-nano/nano-port-keil/an0039-nano-port-keil?id=%e6%b7%bb%e5%8a%a0-rt-thread-nano-%e5%88%b0%e5%b7%a5%e7%a8%8b

    rtthread nano的版本为3.15 2021-06-17发布，使用最新版应该也无影响
## 文件目录
```text

├── prj/                                        # 工程文件夹
│   ├── RTE/                                    # pack中的配置文件与接口文件
│   │   ├── _src                                # 工程配置文件
│   │   ├── Device                              # stm32f103c8相关文件
│   │   └── RTOS/                               # rtthread nano相关文件
│   │       ├── board.c                         # 移植文件(与用户本身的硬件配置有关)
│   │       ├── ···
│   │       └── rtconfig.h                      # 配置文件
│   ├── output/                                 # hex存放文件夹
│   │   └── flow_calib.hex                      # hex文件
│   ├── copy_hex.bat                            # 编译后拷贝的hex脚本
│   └── flow_calib.uvprojx                      # 工程文件
├── src/                                        # 代码文件夹
│   ├── cmsis                                   # cmsis文件与stm32启动有关
│   ├── stm32_lib                               # stm32库文件
│   └── user/                                   # 用户代码文件E:\a_workplace\1\1\flow_calib\work_code\prj\Objects
│       ├── bsp/                                # 底层与传感器无关驱动
│       │   ├── bsp_delay.c                     # 用于iic等通信中的等待延时文件，实现方式与rtos无关
│       │   └── bsp_iic.c                       # 多实例软件模拟iic驱动
│       ├── components/                         # 组件文件夹
│       │   ├── mm_fifo.c                       # fifo文件，采用别人写的库，建议采用本程序中的版本
│       │   │                                   # 库地址：https://github.com/meng-plus/mm_fifo
│       │   └── sf_lib.c                        # 通用函数文件
│       ├── device/                             # 驱动文件夹
│       │   └── XGZP6818D.c                     # 差压传感器驱动
│       ├── service/                            # 服务层函数文件
│       │   ├── calib.c                         # 一次标定与修正标定文件
│       │   ├── first_order_filter.c            # 一阶滤波算法文件
│       │   └── measure_data.c                  # 测量流程
│       ├── flow_calib.c                        # 测量任务
│       ├── main.c                              # 主函数， 运行了一个pc13的运行指示灯，运行周期为1Hz
│       ├── stm32f10x...                        # stm32相关文件
│       └── thread_config.h                     # 工程中的线程配置文件
└── readme.md
```

## todo list
- [x] 一阶滤波算法
- [x] 一次标定以及修正标定算法
- [x] 软件多实例iic编写
- [x] 传感器驱动移植
- [ ] 重构iic模块
- [ ] finsh移植
- [ ] 增加命令行与交互
