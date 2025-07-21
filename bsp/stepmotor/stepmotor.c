#include "stepmotor.h"

// 全局变量用于调试
static uint32_t total_steps_executed = 0;

// 四相八拍步进时序表 (适用于28BYJ-48步进电机)
static const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},  // LA
    {1, 1, 0, 0},  // LA + LB
    {0, 1, 0, 0},  // LB
    {0, 1, 1, 0},  // LB + LC
    {0, 0, 1, 0},  // LC
    {0, 0, 1, 1},  // LC + LD
    {0, 0, 0, 1},  // LD
    {1, 0, 0, 1}   // LD + LA
};

/**
 * @brief  延时函数 (简单延时)
 * @param  ms: 延时毫秒数
 * @retval None
 */
static void Stepper_Delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for(i = 0; i < ms; i++) {
        for(j = 0; j < 8000; j++); // 大约1ms延时 (假设72MHz系统时钟)
    }
}

/**
 * @brief  设置步进电机引脚状态
 * @param  la, lb, lc, ld: 四个引脚的状态 (0或1)
 * @retval None
 */
static void Stepper_SetPins(uint8_t la, uint8_t lb, uint8_t lc, uint8_t ld)
{
    // 设置LA (GPIO_Pin_0)
    if(la) {
        GPIO_SetBits(Stepper_Output_GPIO, Stepper_LA);
    } else {
        GPIO_ResetBits(Stepper_Output_GPIO, Stepper_LA);
    }
    
    // 设置LB (GPIO_Pin_1)
    if(lb) {
        GPIO_SetBits(Stepper_Output_GPIO, Stepper_LB);
    } else {
        GPIO_ResetBits(Stepper_Output_GPIO, Stepper_LB);
    }
    
    // 设置LC (GPIO_Pin_2)
    if(lc) {
        GPIO_SetBits(Stepper_Output_GPIO, Stepper_LC);
    } else {
        GPIO_ResetBits(Stepper_Output_GPIO, Stepper_LC);
    }
    
    // 设置LD (GPIO_Pin_3)
    if(ld) {
        GPIO_SetBits(Stepper_Output_GPIO, Stepper_LD);
    } else {
        GPIO_ResetBits(Stepper_Output_GPIO, Stepper_LD);
    }
}

/**
 * @brief  步进电机初始化
 * @param  None
 * @retval None
 */
void Stepper_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(Stepper_CLK, ENABLE);
    
    // 配置GPIO引脚 PA0-PA3
    GPIO_InitStructure.GPIO_Pin = Stepper_LA | Stepper_LB | Stepper_LC | Stepper_LD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // 50MHz速度
    GPIO_Init(Stepper_Output_GPIO, &GPIO_InitStructure);
    
    // 初始化时停止电机
    Stepper_Stop();
}

/**
 * @brief  步进电机按步数旋转
 * @param  direction: 旋转方向 (Foreward=0逆时针, Reversal=1顺时针) [已修正方向]
 * @param  steps: 步数
 * @param  speed: 速度 (延时ms，建议1-10)
 * @retval None
 */
void Stepper_RotateByStep(RotDirection direction, uint32_t steps, uint16_t speed)
{
    uint32_t i;
    static uint8_t step_index = 0;
    
    for(i = 0; i < steps; i++) {
        if(direction == Reversal) {
            // 原逆时针逻辑改为顺时针
            step_index++;
            if(step_index >= 8) {
                step_index = 0;
            }
        } else {
            // 原顺时针逻辑改为逆时针
            if(step_index == 0) {
                step_index = 7;
            } else {
                step_index--;
            }
        }
        
        // 设置引脚状态
        Stepper_SetPins(step_sequence[step_index][0],
                       step_sequence[step_index][1],
                       step_sequence[step_index][2],
                       step_sequence[step_index][3]);
        
        // 延时控制速度
        Stepper_Delay_ms(speed);
        
        // 计数
        total_steps_executed++;
    }
    
    // 运行完成后停止电机
    Stepper_Stop();
}

/**
 * @brief  步进电机按角度旋转
 * @param  direction: 旋转方向 (Foreward=0逆时针, Reversal=1顺时针) [已修正方向]
 * @param  angle: 角度 (度)
 * @param  speed: 速度 (延时ms，建议1-10)
 * @retval None
 */
void Stepper_RotateByAngle(RotDirection direction, float angle, uint16_t speed)
{
    uint32_t steps;
    
    // 计算需要的步数 
    // 28BYJ-48实际参数: 4096步/圈 (考虑64:1减速比)
    // 实际步进角: 360° ÷ 4096步 = 0.087890625°/步
    steps = (uint32_t)(angle / 0.087890625f);
    
    // 调用按步数旋转函数
    Stepper_RotateByStep(direction, steps, speed);
}

/**
 * @brief  步进电机单步执行 (用于观察每一拍)
 * @param  direction: 旋转方向 (Foreward=0逆时针, Reversal=1顺时针) [已修正方向]
 * @param  speed: 速度 (延时ms，建议1-10)
 * @retval None
 */
void Stepper_SingleStep(RotDirection direction, uint16_t speed)
{
    static uint8_t step_index = 0;
    
    if(direction == Reversal) {
        // 原逆时针逻辑改为顺时针
        step_index++;
        if(step_index >= 8) {
            step_index = 0;
        }
    } else {
        // 原顺时针逻辑改为逆时针
        if(step_index == 0) {
            step_index = 7;
        } else {
            step_index--;
        }
    }
    
    // 设置引脚状态
    Stepper_SetPins(step_sequence[step_index][0],
                   step_sequence[step_index][1],
                   step_sequence[step_index][2],
                   step_sequence[step_index][3]);
    
    // 延时控制速度
    Stepper_Delay_ms(speed);
}

/**
 * @brief  停止步进电机
 * @param  None
 * @retval None
 */
void Stepper_Stop(void)
{
    // 断电停止，所有引脚设为低电平
    Stepper_SetPins(0, 0, 0, 0);
}
