# 28BYJ-48步进电机控制系统

基于STM32F103C8T6的高精度步进电机控制系统，支持四相八拍驱动、串口交互控制和教学演示功能。

*本项目基于立创开发板(LCKFB)开源模板开发*

![STM32](https://img.shields.io/badge/MCU-STM32F103C8T6-blue)
![Motor](https://img.shields.io/badge/Motor-28BYJ--48-green)
![Language](https://img.shields.io/badge/Language-C-yellow)
![IDE](https://img.shields.io/badge/IDE-Keil%20MDK5-red)
![Template](https://img.shields.io/badge/Template-LCKFB-orange)

## 📋 项目特点

- ✅ **精确角度控制** - 修正了28BYJ-48的真实参数(4096步/圈)
- ✅ **四相八拍驱动** - 高转矩、低振动的驱动方式
- ✅ **双模式操作** - 快速控制模式 + 教学演示模式
- ✅ **串口交互** - 丰富的命令集，实时控制
- ✅ **节拍观察** - 缓慢显示每一拍的详细状态
- ✅ **参数可调** - 支持速度调节和多种测试模式

## 🔧 硬件要求

### 主控制器
- **MCU**: STM32F103C8T6 (立创开发板最小系统)
- **开发板**: 立创开发板 (LCKFB)
- **时钟**: 72MHz系统时钟
- **调试**: ST-Link V2

### 步进电机
- **型号**: 28BYJ-48 (5V)
- **驱动板**: ULN2003
- **参数**: 
  - 步进角: 5.625°/64步 (电机轴)
  - 减速比: 1:64
  - 输出轴精度: 0.0879°/步

## 📐 引脚连接

```
28BYJ-48 + ULN2003驱动板连接:
┌─────────────┬──────────────┐
│  驱动板引脚  │  STM32引脚   │
├─────────────┼──────────────┤
│    IN1      │     PA4      │
│    IN2      │     PA5      │
│    IN3      │     PA6      │
│    IN4      │     PA7      │
│    VCC      │     5V       │
│    GND      │     GND      │
└─────────────┴──────────────┘

串口通信引脚 (UART1):
┌─────────────┬──────────────┐
│   功能      │  STM32引脚   │
├─────────────┼──────────────┤
│    TX       │     PA9      │
│    RX       │     PA10     │
└─────────────┴──────────────┘

状态指示:
PC13 → 板载LED (运行状态指示)
```

## 🚀 快速开始

### 1. 环境准备
```bash
# 工具链要求
- Keil MDK5 (v5.37+)
- STM32F1xx标准外设库
- ST-Link Utility
```

### 2. 编译下载
1. 使用Keil MDK5打开项目文件 `Project.uvprojx`
2. 编译项目 (F7)
3. 连接ST-Link下载到STM32F103C8T6
4. 复位运行

### 3. 串口调试
```bash
波特率: 115200
数据位: 8
停止位: 1
校验位: None
```

**⚠️ 中文显示问题解决**
如果串口中文显示为乱码：
- 串口工具编码设置为 `UTF-8` 或 `GBK`
- 关闭HEX显示模式
- 推荐使用：SSCOM、串口精灵、XCOM V2.0

## 🎮 控制命令

### 基本控制
| 命令 | 功能 | 说明 |
|------|------|------|
| `1` | 顺时针转90° | 精确转动1024步 |
| `2` | 逆时针转90° | 精确转动1024步 |
| `3` | 顺时针转360° | 完整一圈转动 |
| `4` | 逆时针转360° | 完整一圈转动 |
| `5` | 停止电机 | 断电停止 |

### 速度控制
| 命令 | 功能 | 延时 |
|------|------|------|
| `s` | 低速模式 | 10ms/步 |
| `f` | 高速模式 | 2ms/步 |

### 测试功能
| 命令 | 功能 | 说明 |
|------|------|------|
| `t` | 测试512步 | 转动45°，验证精度 |
| `d` | 演示模式 | 缓慢显示四相八拍 |
| `q` | 退出演示 | 回到正常控制模式 |

## 📊 技术参数

### 角度精度
```
理论精度: 0.087890625°/步
常用角度转换:
- 90° = 1024步
- 45° = 512步  
- 180° = 2048步
- 360° = 4096步
```

### 四相八拍时序
```
步序  PA0  PA1  PA2  PA3  励磁相    说明
 1    1    0    0    0     A      A相励磁
 2    1    1    0    0    A+B     A+B相同时励磁
 3    0    1    0    0     B      B相励磁
 4    0    1    1    0    B+C     B+C相同时励磁
 5    0    0    1    0     C      C相励磁
 6    0    0    1    1    C+D     C+D相同时励磁
 7    0    0    0    1     D      D相励磁
 8    1    0    0    1    D+A     D+A相同时励磁
```

## 🎓 演示模式

发送命令 `d` 进入教学演示模式，系统将：

### 实时显示
- 每1秒执行一步
- 显示PA0-PA3引脚状态
- 说明当前励磁相
- 统计周期和角度

### 输出示例
```
--- 第1步 ---
引脚状态: PA0=1 PA1=0 PA2=0 PA3=0
节拍序号: 1/8
说明: A相励磁

--- 第8步 ---
引脚状态: PA0=1 PA1=0 PA2=0 PA3=1
节拍序号: 8/8
说明: D+A相同时励磁
*** 完成一个八拍周期 (0.703度) ***
```

## 📁 项目结构

```
STM32F103C8T6_ProjectTemplate/          # 基于立创开发板项目模板
├── app/
│   └── main.c              # 主程序文件
├── bsp/                    # 板级支持包 (LCKFB模板结构)
│   ├── stepmotor/
│   │   ├── stepmotor.h     # 步进电机头文件
│   │   └── stepmotor.c     # 步进电机驱动实现
│   └── uart/
│       ├── bsp_uart.h      # 串口通信头文件 (LCKFB)
│       └── bsp_uart.c      # 串口通信实现 (LCKFB)
├── board/
│   └── board.c             # 板级初始化 (LCKFB)
├── libraries/              # STM32标准外设库
└── project/MDK(V5)/
    └── Project.uvprojx     # Keil项目文件
```

## 🔍 核心函数

### 初始化函数
```c
void Stepper_Init(void);                    // 步进电机初始化
```

### 控制函数
```c
void Stepper_RotateByAngle(RotDirection direction, float angle, uint16_t speed);
void Stepper_RotateByStep(RotDirection direction, uint32_t steps, uint16_t speed);
void Stepper_SingleStep(RotDirection direction, uint16_t speed);
void Stepper_Stop(void);
```

### 参数定义
```c
typedef enum {
    Foreward = 0,    // 顺时针
    Reversal = 1     // 逆时针
} RotDirection;
```

## 🛠️ 问题解决

### 常见问题

**Q: 电机不转动？**
- 检查5V电源是否充足 (≥200mA)
- 验证PA0-PA3引脚连接
- 确认ULN2003驱动板状态

**Q: 角度不准确？**
- 已修正为4096步/圈参数
- 检查机械负载是否过重
- 确认无失步现象

**Q: 运行不稳定？**
- 降低运行速度 (`s`命令)
- 检查电源纹波
- 确认硬件连接可靠

### 性能优化
- 推荐速度: 2-10ms/步
- 高速模式: 适用于轻负载
- 低速模式: 适用于重负载或高精度

## 📈 扩展功能

- [ ] 加减速控制算法
- [ ] 多电机同步控制
- [ ] 编码器位置反馈
- [ ] S型速度曲线
- [ ] 路径规划算法

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 🤝 贡献

欢迎提交Issue和Pull Request！

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 👨‍💻 作者

- **您的姓名** - *初始工作* - [您的GitHub](https://github.com/yourusername)

## 🙏 致谢

- **立创开发板(LCKFB)** - 提供开源硬件平台和项目模板
  - 开发板官网：[www.lckfb.com](https://www.lckfb.com)
  - 立创论坛：[oshwhub.com/forum](https://oshwhub.com/forum)
  - Bilibili：[@立创开发板](https://space.bilibili.com/507078524)
- STMicroelectronics - STM32标准外设库
- 28BYJ-48步进电机开源社区

---

⭐ 如果这个项目对您有帮助，请给个Star支持一下！
