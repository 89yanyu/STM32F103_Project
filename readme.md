### 本项目集合了所有基于STM32F103的工程
+ 所有工程基于STM32F1标准库和自己开发的一些库

### 项目的目录树
    ├─CORE (Cortex相关)
    ├─FWLIB (STM32标准库)
    │  ├─inc
    │  └─src
    ├─MYLIB (自己开发的库)
    ├─OBJ (生成的目标文件)
    └─USER (该项目特有代码)
 
### 项目功能
+ BluetoothTest：双轮小车项目（[上位机代码](https://github.com/89yanyu/Qt_Project/tree/master/MC)）
+ MC：PID电机控制（[上位机代码](https://github.com/89yanyu/Qt_Project/tree/master/MC)）
+ 其他基本都是用于测试或者单独模块开发
+ Template为模板项目，包含所有库以及默认工程文件
    
###### 因为是个人代码，所以几乎没有注释。但是，
### BluetoothTest中所有代码均有注释

### 除MC及其相关项目，其他项目的管脚分配均基于正点原子战舰V3开发板
## 由于时间久远，并不能保证所有代码的正确性。

### 主要自己开发的库
+ beep：控制响铃
+ delay：利用SysTick的精确延时和循环模拟的延时（仿照正点原子）
+ encoder：编码器（双轮小车项目中改为左右两个文件）
+ exinterrupt：外部中断
+ i2c：I2C协议（主要用于MPU6050通讯）
+ key：按键响应，包括按键中断
+ led：LED灯
+ mpu6050：6轴运动处理组件
+ pwm：利用定时器的PWM模式生成PWM波
+ timer：定时器及其中断
+ usart：USB串口（USART1），一般用于打印调试信息，或读取来自上位机的可见内容
+ wifi：串口转wifi（USART2），与wifi模块（ESP8266）通信，采用AT指令，支持超时返回
+ bluetooth：串口转蓝牙（USART3），通过蓝牙与上位机通讯，采用自定义的通讯协议，以保证数据的正确性，完整性。不支持同时存储多条数据

### 如有问题或建议可联系 995239101(你懂得)qq.com
