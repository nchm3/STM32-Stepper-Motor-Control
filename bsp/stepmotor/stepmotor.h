 #ifndef __STEPMOTOR_H__
 #define __STEPMOTOR_H__
 
 #include "stm32f10x.h"
 
// 电机的旋转方向
typedef enum 
{
	Foreward = 0,
	Reversal = 1
} RotDirection;

// 需要使用其他端口时，只需要更改以下的宏定义即可
// 这里需要保证四个输出端口同属一个GPIO
// 如果不能满足这一点，需要更改Stepper.c中初始化函数Stepper_Init和Stepper_RotateByStep中的一些变量名称
// 这里的宏定义是为了提高程序的可读性和可移植性，但使用stm32f10x.h中定义的原始名称也未尝不可
#define		Stepper_CLK				RCC_APB2Periph_GPIOA
#define		Stepper_Output_GPIO		GPIOA
#define 	Stepper_LA				GPIO_Pin_4
#define 	Stepper_LB				GPIO_Pin_5
#define 	Stepper_LC				GPIO_Pin_6
#define 	Stepper_LD				GPIO_Pin_7

// 函数声明
void Stepper_Init(void);
void Stepper_RotateByStep(RotDirection direction, uint32_t steps, uint16_t speed);
void Stepper_RotateByAngle(RotDirection direction, float angle, uint16_t speed);
void Stepper_Stop(void);

#endif
