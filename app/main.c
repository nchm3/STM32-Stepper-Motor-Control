#include "stm32f10x.h"
#include "board.h"
#include "bsp_uart.h"
#include "stdio.h"
#include "stepmotor.h"

// ��ӵ�����������
void Stepper_SingleStep(RotDirection direction, uint16_t speed);

int main(void)
{
	
	board_init();
	
	uart1_init(115200);
	
	// ��ʼ���������
	Stepper_Init();
	
	printf("\r\n========================================\r\n");
	printf("     28BYJ-48�����������ϵͳ\r\n");
	printf("========================================\r\n");
	printf("�������: 4096��/Ȧ, 0.0879��/��\r\n");
	printf("�ǶȾ�����֤:\r\n");
	printf("  90�� = %d��\r\n", (uint32_t)(90.0 / 0.087890625f));
	printf("  45�� = %d��\r\n", (uint32_t)(45.0 / 0.087890625f));
	printf("  360�� = %d��\r\n", (uint32_t)(360.0 / 0.087890625f));
	printf("========================================\r\n");
	printf("��������:\r\n");
	printf("  1 - ˳ʱ��ת90��\r\n");
	printf("  2 - ��ʱ��ת90��\r\n");
	printf("  3 - ˳ʱ��תһȦ\r\n");
	printf("  4 - ��ʱ��תһȦ\r\n");
	printf("  5 - ֹͣ���\r\n");
	printf("  s - ����ģʽ(10ms)\r\n");
	printf("  f - ����ģʽ(2ms)\r\n");
	printf("  t - ����512��\r\n");
	printf("  d - ��ʾģʽ(������ʾ����)\r\n");
	printf("  q - �˳���ʾģʽ\r\n");
	printf("========================================\r\n");
	
	// ��ʼ��LED
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);

	// ���Ʊ���
	uint16_t motor_speed = 3;          // Ĭ���ٶ� 3ms
	uint8_t demo_mode = 0;             // ��ʾģʽ��־
	uint32_t demo_step_count = 0;      // ��ʾģʽ��������
	uint32_t demo_timer = 0;           // ��ʾģʽ��ʱ��
	
	printf("ϵͳ�������뷢������...\r\n");
	
	while(1)
	{
		// LED״ָ̬ʾ
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		delay_ms(50);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		delay_ms(50);
		
		// ��ʾģʽ - ������ʾ����
		if(demo_mode) {
			demo_timer++;
			
			// ÿ1��ִ��һ��
			if(demo_timer >= 10) {  // 100ms * 10 = 1��
				demo_timer = 0;
				demo_step_count++;
				
				printf("\r\n--- ��%d�� ---\r\n", demo_step_count);
				
				// ִ��һ��
				Stepper_SingleStep(Foreward, 1);  // 1ms��ʱ
				
				// ��ȡ����ʾ��ǰ����״̬
				uint8_t pa4_state = (GPIOA->ODR & GPIO_Pin_4) ? 1 : 0;
				uint8_t pa5_state = (GPIOA->ODR & GPIO_Pin_5) ? 1 : 0;
				uint8_t pa6_state = (GPIOA->ODR & GPIO_Pin_6) ? 1 : 0;
				uint8_t pa7_state = (GPIOA->ODR & GPIO_Pin_7) ? 1 : 0;
				
				printf("����״̬: PA4=%d PA5=%d PA6=%d PA7=%d\r\n", 
				       pa4_state, pa5_state, pa6_state, pa7_state);
				printf("�������: %d/8\r\n", (demo_step_count-1)%8 + 1);
				
				// ��ʾ�������˵��
				uint8_t beat = (demo_step_count-1)%8;
				switch(beat) {
					case 0: printf("˵��: A������\r\n"); break;
					case 1: printf("˵��: A+B��ͬʱ����\r\n"); break;
					case 2: printf("˵��: B������\r\n"); break;
					case 3: printf("˵��: B+C��ͬʱ����\r\n"); break;
					case 4: printf("˵��: C������\r\n"); break;
					case 5: printf("˵��: C+D��ͬʱ����\r\n"); break;
					case 6: printf("˵��: D������\r\n"); break;
					case 7: printf("˵��: D+A��ͬʱ����\r\n"); break;
				}
				
				// ÿ8��(һ����������)��ʾ�ܽ�
				if(demo_step_count % 8 == 0) {
					printf("*** ���һ���������� (%.3f��) ***\r\n", 
					       8 * 0.087890625f);
				}
				
				// ÿ64����ʾһ�������ĵ����ת��
				if(demo_step_count % 64 == 0) {
					printf("=== ���64�� (�����5.625��) ===\r\n");
				}
				
				// ÿ4096����ʾһȦ
				if(demo_step_count >= 4096) {
					printf("�������� ���4096�� (�����һȦ360��) ��������\r\n");
					demo_step_count = 0;
					printf("���¿�ʼ����...\r\n");
				}
				
				printf("���� 'q' �˳���ʾģʽ\r\n");
			}
		}
		
		// ���������
		uint8_t *t = uart1_get_data();
		if(t != NULL)
		{
			printf("\r\n���յ�����: %s\r\n", t);
			
			if(t[0] == '1') {
				printf("ִ��: ˳ʱ��ת90��\r\n");
				demo_mode = 0;  // �˳���ʾģʽ
				Stepper_RotateByAngle(Foreward, 90.0, motor_speed);
				printf("���!\r\n");
			}
			else if(t[0] == '2') {
				printf("ִ��: ��ʱ��ת90��\r\n");
				demo_mode = 0;  // �˳���ʾģʽ
				Stepper_RotateByAngle(Reversal, 90.0, motor_speed);
				printf("���!\r\n");
			}
			else if(t[0] == '3') {
				printf("ִ��: ˳ʱ��תһȦ(360��)\r\n");
				demo_mode = 0;  // �˳���ʾģʽ
				Stepper_RotateByAngle(Foreward, 360.0, motor_speed);
				printf("���!\r\n");
			}
			else if(t[0] == '4') {
				printf("ִ��: ��ʱ��תһȦ(360��)\r\n");
				demo_mode = 0;  // �˳���ʾģʽ
				Stepper_RotateByAngle(Reversal, 360.0, motor_speed);
				printf("���!\r\n");
			}
			else if(t[0] == '5') {
				printf("ִ��: ֹͣ���\r\n");
				demo_mode = 0;  // �˳���ʾģʽ
				Stepper_Stop();
				printf("�����ֹͣ!\r\n");
			}
			else if(t[0] == 's') {
				printf("����: ����ģʽ(10ms��ʱ)\r\n");
				motor_speed = 10;
			}
			else if(t[0] == 'f') {
				printf("����: ����ģʽ(2ms��ʱ)\r\n");
				motor_speed = 2;
			}
			else if(t[0] == 't') {
				printf("ִ��: ����512��\r\n");
				demo_mode = 0;  // �˳���ʾģʽ
				Stepper_RotateByStep(Foreward, 512, motor_speed);
				printf("�������! (ת���Ƕ�: %.1f��)\r\n", 512 * 0.087890625f);
			}
			else if(t[0] == 'd') {
				printf("����: ��ʾģʽ - ������ʾ�������\r\n");
				printf("ÿ1��ִ��һ������ʾ��ϸ�Ľ�����Ϣ\r\n");
				printf("�۲�PA4-PA7����״̬�仯...\r\n");
				demo_mode = 1;
				demo_step_count = 0;
				demo_timer = 0;
			}
			else if(t[0] == 'q') {
				if(demo_mode) {
					printf("�˳�: ��ʾģʽ\r\n");
					demo_mode = 0;
					Stepper_Stop();
				} else {
					printf("��ǰ������ʾģʽ\r\n");
				}
			}
			else {
				printf("δ֪����: %c\r\n", t[0]);
				printf("��������: 1,2,3,4,5,s,f,t,d,q\r\n");
			}
			
			if(!demo_mode) {
				printf("���������� (����'d'������ʾģʽ):\r\n");
			}
		}
	}
}
