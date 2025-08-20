# CAN_DWIN
智能汽车CAN通信仪表盘串口显示屏


**开发方式**  RT-Thread_SDK + ENV + STM32CubeMX + MDK5 + DGUS（迪文屏GUI开发工具）

**项目基于RT-Thread嵌入式实时操作系统开发**，实现了智能汽车中CAN通信协议与串口屏协议之间的数据转换与可视化展示。系统通过解析车载CAN总线数据，将关键运行信息（如指示灯、车速、加速度等）以图形化界面实时展示在迪文屏上。

**项目结构图**


<img width="516" height="258" alt="image" src="https://github.com/user-attachments/assets/282606e5-c433-4a7f-b6fa-2fbb66a3508f" />

**本项目主要**操作迪文屏的协议转换版，此MCU的型号为：STM32F412RET6 ； 主要操作外设为USART1/3 ，CAN1 


<img width="872" height="652" alt="image" src="https://github.com/user-attachments/assets/b42eeb9e-ad0c-4862-8c7b-70626bccafdd" />
