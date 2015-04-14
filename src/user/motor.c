#include "stm32f10x.h"
#include "delay.h"
#include "main.h"
#include "util.h"

#define Max_Speed 5000
#define LEFT_LOW_ON()   GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define LEFT_LOW_OFF()  GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define RIGHT_LOW_ON()  GPIO_SetBits(GPIOA,GPIO_Pin_10)
#define RIGHT_LOW_OFF() GPIO_ResetBits(GPIOA,GPIO_Pin_10)

int8_t motor_enable = 0;

void Motor_Init(void)
{
	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	LEFT_LOW_OFF();
	RIGHT_LOW_OFF();
	TIM1->CCR2 = 0;
	TIM1->CCR4 = 0;
}
void Motor_Speed(int32_t speed)
{
	static int32_t last_speed = 0;

	if (!motor_enable)
		speed = 0;
	if (abs(speed) > Max_Speed)
	{
		if (speed < 0)
			speed = -Max_Speed;
		else if (speed > 0)
			speed = Max_Speed;
	}            //�����ٶ�,�涨��ǰΪ��

	if (speed > 0)   //�����תΪ��ǰ
	{
		if (last_speed <= 0)
		{
			TIM1->CCR4 = 0;
			delay_us(100);
			LEFT_LOW_OFF();
			delay_us(100);
			RIGHT_LOW_ON();
			delay_us(100);
			TIM1->CCR2 = speed;
		}
		else
		{
			LEFT_LOW_OFF();
			RIGHT_LOW_ON();
			TIM1->CCR2 = speed;
		}
	}

	else if (speed < 0)
	{
		if (last_speed >= 0)
		{
			TIM1->CCR2 = 0;
			delay_us(100);
			RIGHT_LOW_OFF();
			delay_us(100);
			LEFT_LOW_ON();
			delay_us(100);
			TIM1->CCR4 = -speed;
		}
		else
		{
			RIGHT_LOW_OFF();
			LEFT_LOW_ON();
			TIM1->CCR4 = -speed;
		}
	}
	else
	{
		TIM1->CCR2 = 0;
		TIM1->CCR4 = 0;
		LEFT_LOW_OFF();
		RIGHT_LOW_OFF();
	}

	last_speed = speed;
}

float Get_MxMi(int num, int max, int min)
{
	if (num > max)
		return max;
	else if (num < min)
		return min;
	else
		return num;
}

int vP = 10, vI = 2, vD = 0;

void Motor_velocity_control(int current_speed, int want_speed)
{
	int error, error_d;
	int32_t output;
	static int error_i = 0, last_error = 0;

	error = want_speed - current_speed;
	error_i += error;
	error_i = Get_MxMi(error_i, 1000.0, -1000.0);
	error_d = error - last_error;
	last_error = error;
	output = vP * error + vI * error_i + vD * error_d;
	output = Get_MxMi(output, 4500.0, -4500.0);
	Motor_Speed(output);

}