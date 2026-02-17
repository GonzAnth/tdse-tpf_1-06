/**
 * @file    task_adc.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Tarea asociada a la máquina de estados del ADC.
 * @date    Jan 24, 2026
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
#include "task_system_attribute.h"
#include "task_system_interface.h"

/********************** macros and definitions *******************************/
#define G_TASK_ADC_CNT_INI			0ul
#define G_TASK_ADC_TICK_CNT_INI		0ul

#define DEL_ADC_XX_MIN				0ul
#define DEL_ADC_XX_MED				50ul
#define DEL_ADC_XX_MAX				500ul

extern ADC_HandleTypeDef hadc1;

/********************** internal data declaration ****************************/
task_adc_cfg_t task_adc_cfg = {
	DEL_ADC_XX_MIN, false
};

task_adc_dta_t task_adc_dta = {
	.state		= ST_ADC_IDLE,
	.event 		= EV_ADC_IDLE,
	.flag_ready	= false
};

/********************** internal functions declaration ***********************/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	task_adc_dta.flag_ready = true;
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
					if ((true == p_task_adc_cfg->flag) && (EV_ADC_START == p_task_adc_dta->event))
					{
						p_task_adc_cfg->flag = false;
						p_task_adc_dta->flag_ready= false;

						HAL_ADC_Start_DMA(&hadc1, (uint32_t*)p_task_adc_dta->adc_buffer, 2);
						p_task_adc_dta->state = ST_ADC_WAITING;

					}

					break;

				case ST_ADC_WAITING:
					if (true == p_task_adc_dta->flag_ready)
					{
						p_task_adc_dta->temp_raw = (uint32_t)p_task_adc_dta->adc_buffer[0];
						p_task_adc_dta->bat_raw  = (uint32_t)p_task_adc_dta->adc_buffer[1];


						if ((p_task_adc_dta->bat_raw == ADC_MIN_COUNT) || (p_task_adc_dta->temp_raw == ADC_MAX_COUNT)) 
						{
							put_event_task_system(EV_SYS_ADC_NOT_OK);
							p_task_adc_dta->state = ST_ADC_FALLA;
							break;
						}

						p_task_adc_dta->bat_volts = VREFINT_CAL_VOLTS * ADC_MAX_COUNT / (float)p_task_adc_dta->bat_raw;

						float temp_v_read = ((float)p_task_adc_dta->temp_raw * p_task_adc_dta->bat_volts) / ADC_MAX_COUNT;
						p_task_adc_dta->temp_cent = (TEMP_V25 - temp_v_read) / TEMP_AVG_SLOPE + 25.0f;

						put_event_task_system(EV_SYS_ADC_OK);
						p_task_adc_dta->state = ST_ADC_IDLE;
					}

					break;

				case ST_ADC_FALLA:

					if ((true == p_task_adc_cfg->flag) && (EV_ADC_FALLA_OK == p_task_adc_dta->event))
					{
						p_task_adc_cfg->flag = false;
						p_task_adc_dta->state = ST_ADC_IDLE;
					}

					break;

				default:
					p_task_adc_cfg->tick  = DEL_ADC_XX_MIN;
					p_task_adc_dta->state = ST_ADC_IDLE;
					p_task_adc_dta->event = EV_ADC_IDLE;
					p_task_adc_cfg->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/

