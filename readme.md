# ReadMe
## 依赖包与编译环境

- **1、编译环境**  
    本工程使用的编译环境以及设置如下
    | 名称 | 版本 |
    |------|------|
    | keil | 5.43 |
    | c complier | 6.24 |
    | assembler | 6.24 |
    | linker/locator | 6.24 |
    | library manager | 6.24 |
    | hex converter | 6.24 |
    
    - **Note**：**请使用keil 5版本编译此工程并且选择编译器为acc6而非acc5**
- **2、依赖包**  
    本工程依赖rtthread nano版本，github链接如下  
    [rtthread nano](https://github.com/RT-Thread/rtthread-nano)  

    本工程使用的软件包如下
    | 名称 | 版本 |
    |------|------|
    | RT-Thread nano | 4.1.1 |
    | CmBacktrace | 1.4.1 |
    - **1、rt thead nano**  
    rt thread nano中需要关注的是bsp文件夹下面的rtconfig.h中的相关配置，其中设置main函数线程的线程栈大小RT_MAIN_THREAD_STACK_SIZE需要设置为512，否则会引发硬件异常  
    board.c文件中包含与硬件配置以及rt thread nano不同配置下的初始化等流程相关  
    - **2、finsh组件**  
    rt thread nano finsh组件进行了部分调整与修改，加入了finsh_config.h文件。  
    finsh_config.h源文件文件目录  src\rt_thread_nano\rt-thread\bsp\stm32f407-msh\Middlewares\Third_Party\RealThread_RTOS\components\finsh  
    - **3、CmBacktrace组件**  
    本组件是用于在硬件异常的情况下进行问题排查的，使用本组件包需要将工程中的对应文件加入编译，并且开启RT_DEBUG宏  
    RT_DEBUG宏位置在rtconfig.h中的37行，注意请不要打开本组件配置文件中的中文选项，会造成乱码




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
│   ├── lib                                     # 使用的第三方库
│   │   └── CmBacktrace-1.4.1/                  # 追踪硬件异常的库
│   ├── rt_thread_nano/                         # rt thread nano 文件
│   │   └── rt-thread/                          # rt thread nano 文件
│   │       ├── bsp                             # rt thread nano bsp层文件
│   │       │   ├── ...                         
│   │       │   ├── board.c                     # 与自身工程配置与硬件配置相关的移植接口文件
│   │       │   └── rtconfig.h                  # rt thread nano配置文件
│   │       ├── components/                     # rt thread nano组件库文件
│   │       │   └── finsh                       # rt thread nano finsh组件文件
│   │       ├── ...                             
│   │       └── src                             # 核心代码文件
│   ├── rt_thread_nano                          # rt thread nano 文件
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
├── cmd_use.txt                                 # msh交互命令使用说明
└── readme.md                                   # read me
```

## todo list
- [x] 一阶滤波算法
- [x] 一次标定以及修正标定算法
- [x] 软件多实例iic编写
- [x] 传感器驱动移植
- [x] finsh移植
- [ ] 重构iic模块
- [ ] 增加命令行与交互
