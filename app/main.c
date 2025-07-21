#include "stm32f10x.h"
#include "board.h"
#include "bsp_uart.h"
#include "stdio.h"
#include "stepmotor.h"

// 添加单步函数声明
void Stepper_SingleStep(RotDirection direction, uint16_t speed);

int main(void)
{
	
	board_init();
	
	uart1_init(115200);
	
	// 初始化步进电机
	Stepper_Init();
	
	printf("\r\n========================================\r\n");
	printf("     28BYJ-48步进电机测试系统\r\n");
	printf("========================================\r\n");
	printf("电机参数: 4096步/圈, 0.0879度/步\r\n");
	printf("角度精度验证:\r\n");
	printf("  90度 = %d步\r\n", (uint32_t)(90.0 / 0.087890625f));
	printf("  45度 = %d步\r\n", (uint32_t)(45.0 / 0.087890625f));
	printf("  360度 = %d步\r\n", (uint32_t)(360.0 / 0.087890625f));
	printf("========================================\r\n");
	printf("控制命令:\r\n");
	printf("  1 - 顺时针转90度\r\n");
	printf("  2 - 逆时针转90度\r\n");
	printf("  3 - 顺时针转一圈\r\n");
	printf("  4 - 逆时针转一圈\r\n");
	printf("  5 - 停止电机\r\n");
	printf("  s - 低速模式(10ms)\r\n");
	printf("  f - 高速模式(2ms)\r\n");
	printf("  t - 测试512步\r\n");
	printf("  d - 演示模式(缓慢显示节拍)\r\n");
	printf("  q - 退出演示模式\r\n");
	printf("========================================\r\n");
	
	// 初始化LED
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);

	// 控制变量
	uint16_t motor_speed = 3;          // 默认速度 3ms
	uint8_t demo_mode = 0;             // 演示模式标志
	uint32_t demo_step_count = 0;      // 演示模式步数计数
	uint32_t demo_timer = 0;           // 演示模式定时器
	
	printf("系统就绪，请发送命令...\r\n");
	
	while(1)
	{
		// LED状态指示
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		delay_ms(50);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		delay_ms(50);
		
		// 演示模式 - 缓慢显示节拍
		if(demo_mode) {
			demo_timer++;
			
			// 每1秒执行一步
			if(demo_timer >= 10) {  // 100ms * 10 = 1秒
				demo_timer = 0;
				demo_step_count++;
				
				printf("\r\n--- 第%d步 ---\r\n", demo_step_count);
				
				// 执行一步
				Stepper_SingleStep(Foreward, 1);  // 1ms延时
				
				// 读取并显示当前引脚状态
				uint8_t pa4_state = (GPIOA->ODR & GPIO_Pin_4) ? 1 : 0;
				uint8_t pa5_state = (GPIOA->ODR & GPIO_Pin_5) ? 1 : 0;
				uint8_t pa6_state = (GPIOA->ODR & GPIO_Pin_6) ? 1 : 0;
				uint8_t pa7_state = (GPIOA->ODR & GPIO_Pin_7) ? 1 : 0;
				
				printf("引脚状态: PA4=%d PA5=%d PA6=%d PA7=%d\r\n", 
				       pa4_state, pa5_state, pa6_state, pa7_state);
				printf("节拍序号: %d/8\r\n", (demo_step_count-1)%8 + 1);
				
				// 显示四相八拍说明
				uint8_t beat = (demo_step_count-1)%8;
				switch(beat) {
					case 0: printf("说明: A相励磁\r\n"); break;
					case 1: printf("说明: A+B相同时励磁\r\n"); break;
					case 2: printf("说明: B相励磁\r\n"); break;
					case 3: printf("说明: B+C相同时励磁\r\n"); break;
					case 4: printf("说明: C相励磁\r\n"); break;
					case 5: printf("说明: C+D相同时励磁\r\n"); break;
					case 6: printf("说明: D相励磁\r\n"); break;
					case 7: printf("说明: D+A相同时励磁\r\n"); break;
				}
				
				// 每8步(一个完整周期)显示总结
				if(demo_step_count % 8 == 0) {
					printf("*** 完成一个八拍周期 (%.3f度) ***\r\n", 
					       8 * 0.087890625f);
				}
				
				// 每64步显示一个完整的电机轴转动
				if(demo_step_count % 64 == 0) {
					printf("=== 完成64步 (电机轴5.625度) ===\r\n");
				}
				
				// 每4096步显示一圈
				if(demo_step_count >= 4096) {
					printf("████ 完成4096步 (输出轴一圈360度) ████\r\n");
					demo_step_count = 0;
					printf("重新开始计数...\r\n");
				}
				
				printf("输入 'q' 退出演示模式\r\n");
			}
		}
		
		// 串口命令处理
		uint8_t *t = uart1_get_data();
		if(t != NULL)
		{
			printf("\r\n接收到命令: %s\r\n", t);
			
			if(t[0] == '1') {
				printf("执行: 顺时针转90度\r\n");
				demo_mode = 0;  // 退出演示模式
				Stepper_RotateByAngle(Foreward, 90.0, motor_speed);
				printf("完成!\r\n");
			}
			else if(t[0] == '2') {
				printf("执行: 逆时针转90度\r\n");
				demo_mode = 0;  // 退出演示模式
				Stepper_RotateByAngle(Reversal, 90.0, motor_speed);
				printf("完成!\r\n");
			}
			else if(t[0] == '3') {
				printf("执行: 顺时针转一圈(360度)\r\n");
				demo_mode = 0;  // 退出演示模式
				Stepper_RotateByAngle(Foreward, 360.0, motor_speed);
				printf("完成!\r\n");
			}
			else if(t[0] == '4') {
				printf("执行: 逆时针转一圈(360度)\r\n");
				demo_mode = 0;  // 退出演示模式
				Stepper_RotateByAngle(Reversal, 360.0, motor_speed);
				printf("完成!\r\n");
			}
			else if(t[0] == '5') {
				printf("执行: 停止电机\r\n");
				demo_mode = 0;  // 退出演示模式
				Stepper_Stop();
				printf("电机已停止!\r\n");
			}
			else if(t[0] == 's') {
				printf("设置: 低速模式(10ms延时)\r\n");
				motor_speed = 10;
			}
			else if(t[0] == 'f') {
				printf("设置: 高速模式(2ms延时)\r\n");
				motor_speed = 2;
			}
			else if(t[0] == 't') {
				printf("执行: 测试512步\r\n");
				demo_mode = 0;  // 退出演示模式
				Stepper_RotateByStep(Foreward, 512, motor_speed);
				printf("测试完成! (转动角度: %.1f度)\r\n", 512 * 0.087890625f);
			}
			else if(t[0] == 'd') {
				printf("进入: 演示模式 - 缓慢显示四相八拍\r\n");
				printf("每1秒执行一步，显示详细的节拍信息\r\n");
				printf("观察PA4-PA7引脚状态变化...\r\n");
				demo_mode = 1;
				demo_step_count = 0;
				demo_timer = 0;
			}
			else if(t[0] == 'q') {
				if(demo_mode) {
					printf("退出: 演示模式\r\n");
					demo_mode = 0;
					Stepper_Stop();
				} else {
					printf("当前不在演示模式\r\n");
				}
			}
			else {
				printf("未知命令: %c\r\n", t[0]);
				printf("可用命令: 1,2,3,4,5,s,f,t,d,q\r\n");
			}
			
			if(!demo_mode) {
				printf("请输入命令 (输入'd'进入演示模式):\r\n");
			}
		}
	}
}
