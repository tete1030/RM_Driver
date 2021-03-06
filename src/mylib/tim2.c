#include "stm32f10x.h"
#include "encoder.h"
#include "motor.h"
#include "debug.h"
#include "main.h"
#include "usart2.h"
#include <stdio.h>
#include "stm32f10x_it.h"

void TIM2_Configuration(int32_t sample_interval_ms)   //sample_interval_ms 定时
{
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef nvic;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	uint32_t TIM2_Clock;
	int32_t TIM2_Clock_MHZ;
	int32_t Period;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = ITP_TIM2_PREEMPTION;
	nvic.NVIC_IRQChannelSubPriority = ITP_TIM2_SUB;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	RCC_GetClocksFreq(&RCC_ClocksStatus);
	TIM2_Clock = RCC_ClocksStatus.PCLK1_Frequency * 2; // When APB1 Division is not 1, multiply 2 the clock
	TIM2_Clock_MHZ = TIM2_Clock / 1000000;
	Period = sample_interval_ms * TIM2_Clock / (TIM2_Clock_MHZ * 1000);

	tim.TIM_Prescaler = TIM2_Clock_MHZ - 1;
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_ClockDivision = TIM_CKD_DIV1;
	tim.TIM_Period = Period - 1;

	TIM_TimeBaseInit(TIM2, &tim);
}

void TIM2_Start(void)
{
	TIM_Cmd(TIM2, ENABLE);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}


#define PID_CONTROL_SPEED

void TIM2_IRQHandler(void)
{
	int16_t encoder_speed;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//(((defined CAR_1) && (defined DRIVER_LEFT_END)) || \

#if (((defined CAR_1) && !(defined DRIVER_LEFT_END)) || \
	(defined CAR_2) || \
    ((defined CAR_3) && (defined DRIVER_RIGHT_END)))
		encoder_speed = Encoder_Get_Count() >> 2;
#else
        encoder_speed = -Encoder_Get_Count() >> 2;
#endif
#ifdef PID_CONTROL_SPEED
        Motor_Speed_Control(encoder_speed);
#endif

	}

}
