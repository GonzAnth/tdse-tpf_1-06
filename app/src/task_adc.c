/*
 * task_adc.c
 *
 *  Created on: Jan 24, 2026
 *      Author: Gonzalo
 */


/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_adc_interface.h"
#include "task_adc_attribute.h"


/********************** macros and definitions *******************************/
#define G_TASK_ADC_CNT_INI			0ul
#define G_TASK_ADC_TICK_CNT_INI		0ul

#define DEL_ADC_XX_MIN				0ul
#define DEL_ADC_XX_MED				50ul
#define DEL_ADC_XX_MAX				500ul

extern ADC_HandleTypeDef hadc1;



/********************** internal data declaration ****************************/
task_adc_cfg_t task_adc_cfg = {
	DEL_ADC_XX_MIN, false, 0,
	0, 0, 0, 0
};

task_adc_dta_t task_adc_dta = {
	0, ST_ADC_IDLE, 0, false, 0, 0.0, 0, 0.0
};

/********************** internal functions declaration ***********************/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	task_adc_dta.last_raw_lecture = HAL_ADC_GetValue(&hadc1);
	task_adc_dta.flag_ready = true;
}


static void ADC_select_channel(uint32_t channel){
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	if (channel == ADC_CHANNEL_TEMPSENSOR || channel == ADC_CHANNEL_VREFINT)
	{
		sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	} else {
		sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
	}

	HAL_ADC_ConfigChannel(&hadc1, &sConfig); //No consideramos caso de falla
}


/********************** internal data definition *****************************/
const char *p_task_adc 		= "Task ADC (Internal Sensors)";
const char *p_task_adc_ 	= "Non-Blocking";

/********************** external data declaration ****************************/
uint32_t g_task_adc_cnt;
volatile uint32_t g_task_adc_tick_cnt;


/********************** external functions definition ************************/
void task_adc_init(void *parameters)
{
	task_adc_cfg_t *p_task_adc_cfg;
	task_adc_dta_t *p_task_adc_dta;
	task_adc_st_t	state;
	task_adc_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_sen_init), p_task_adc);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_sen), p_task_adc_);

	g_task_adc_cnt = G_TASK_ADC_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_adc_cnt), g_task_adc_cnt);

	init_queue_event_task_adc();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_adc_dta = &task_adc_dta;
	p_task_adc_cfg = &task_adc_cfg;

	/* Print out: Task execution FSM */
	state = p_task_adc_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_adc_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_adc_cfg->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	cycle_counter_init();
	cycle_counter_reset();

	g_task_adc_tick_cnt = G_TASK_ADC_TICK_CNT_INI;
}

void task_adc_update(void *parameters)
{
	task_adc_cfg_t *p_task_adc_cfg;
	task_adc_dta_t *p_task_adc_dta;

	bool b_time_update_required = false;

	/* Update Task Menu Counter */
	g_task_adc_cnt++;

	/* Protect shared resource (g_task_sen_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_ADC_TICK_CNT_INI < g_task_adc_tick_cnt)
    {
    	g_task_adc_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_sen_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_ADC_TICK_CNT_INI < g_task_adc_tick_cnt)
		{
			g_task_adc_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_adc_cfg = &task_adc_cfg;
		p_task_adc_dta = &task_adc_dta;

    	if (DEL_ADC_XX_MIN < p_task_adc_cfg->tick)
		{
			p_task_adc_cfg->tick--;
		}
		else
		{
			p_task_adc_cfg->tick = DEL_ADC_XX_MAX;

			 /*Implementacion maquina de estados */
			if (true == any_event_task_adc())
			{
				p_task_adc_cfg->flag = true;
				p_task_adc_dta->event = get_event_task_adc();
			}

			switch (p_task_adc_dta->state)
			{
				case ST_ADC_IDLE:
					p_task_adc_dta->tick_adc++;
					if (p_task_adc_dta->tick_adc >= 30)
					{
						p_task_adc_dta->tick_adc = 0;
						p_task_adc_dta->state = ST_ADC_TEMP_START;
					}
					break;


				case ST_ADC_TEMP_START:
					p_task_adc_dta->flag_ready = false;
					ADC_select_channel(ADC_CHANNEL_TEMPSENSOR);
					HAL_ADC_Start_IT(&hadc1);

					p_task_adc_dta->state = ST_ADC_TEMP_WAITING;
					break;


				case ST_ADC_TEMP_WAITING:
					if (p_task_adc_dta->flag_ready == true)
					{
						p_task_adc_dta->temp_raw = p_task_adc_dta->last_raw_lecture;
						if (p_task_adc_dta->temp_raw == 0)
						{
							p_task_adc_dta->state = ST_ADC_FALLA;
						}
						p_task_adc_dta->state = ST_ADC_BAT_START;
					}

					break;


				case ST_ADC_BAT_START:
					p_task_adc_dta->flag_ready = false;
					ADC_select_channel(ADC_CHANNEL_VREFINT);
					HAL_ADC_Start_IT(&hadc1);

					p_task_adc_dta->state = ST_ADC_BAT_WAITING;
					break;


				case ST_ADC_BAT_WAITING:
					if (p_task_adc_dta->flag_ready == true)
					{
						p_task_adc_dta->bat_raw = p_task_adc_dta->last_raw_lecture;


						if (p_task_adc_dta->bat_raw > 0)
						{
							p_task_adc_dta->bat_volts = VREFINT_CAL_VOLTS * ADC_MAX_COUNT / (float)p_task_adc_dta->bat_raw;
						} else {
							p_task_adc_dta->state = ST_ADC_FALLA;
						}

						float temp_v_read = ( (float)p_task_adc_dta->temp_raw * p_task_adc_dta->bat_volts ) / ADC_MAX_COUNT;
						p_task_adc_dta->temp_cent = (TEMP_V25 - temp_v_read)/TEMP_AVG_SLOPE + 25.0f;

						p_task_adc_dta->state = ST_ADC_IDLE;
					}

					break;

				case ST_ADC_FALLA:
					//TODO: Manejo de error y put event en system
					p_task_adc_dta->state = ST_ADC_IDLE;
					break;

				default:
					p_task_adc_cfg->tick  = DEL_ADC_XX_MIN;
					p_task_adc_dta->state = ST_ADC_IDLE;
					p_task_adc_dta->event = 0;
					p_task_adc_cfg->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/

