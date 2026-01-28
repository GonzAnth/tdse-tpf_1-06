/**
 * @file    task_system.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Tarea asociada a la máquina de estados del Systema.
 * @date    Jan 10, 2026
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
#include "display.h"
#include "task_sht85_attribute.h"
#include "task_sht85_interface.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "task_adc_attribute.h"
#include "task_adc_interface.h"


/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI			0ul
#define G_TASK_SYS_TICK_CNT_INI		0ul

#define DEL_SYS_XX_MIN				0ul
#define DEL_SYS_XX_MED				50ul
#define DEL_SYS_XX_MAX				500ul

#define DEL_SYS_IDLE_MAX			50ul
#define DEL_SYS_IDLE_MIN			0ul

#define DEL_SYS_RIEGO_MAX			20ul
#define DEL_SYS_RIEGO_MIN			0ul

#define DEL_SYS_FALLA_MAX			5ul
#define DEL_SYS_FALLA_MIN			0ul

#define THRESHOLD_SYS_TEMP_DEF		24ul
#define THRESHOLD_SYS_HUM_DEF		30ul

#define THRESHOLD_SYS_ADC_TEMP_DEF		80ul
#define THRESHOLD_SYS_ADC_BAT_DEF		1ul

/********************** internal data declaration ****************************/
task_system_cfg_t task_system_cfg = {
	DEL_SYS_XX_MIN, false, DEL_SYS_IDLE_MAX, DEL_SYS_RIEGO_MAX, DEL_SYS_FALLA_MAX, true, THRESHOLD_SYS_TEMP_DEF, THRESHOLD_SYS_HUM_DEF,
	THRESHOLD_SYS_ADC_TEMP_DEF, THRESHOLD_SYS_ADC_BAT_DEF,
	0, 0,
	EV_SEN_MEASURE_ON, EV_SEN_MEASURE_READ, EV_SEN_FALLA_OK,
	EV_ADC_START,
	EV_MEN_ADC_REQ_OK
};

task_system_dta_t task_system_dta = {
	DEL_SYS_IDLE_MAX, DEL_SYS_RIEGO_MIN,DEL_SYS_FALLA_MIN, ST_SYS_IDLE, EV_SYS_RIEGO_NACT_ON, 0.0, 0.0, 0.0, 0.0
};

#define SYSTEM_DTA_QTY	(sizeof(task_system_dta)/sizeof(task_system_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_system 		= "Task Menu (Interactive Menu)";
const char *p_task_system_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_system_cnt;
volatile uint32_t g_task_system_tick_cnt;

/********************** external functions definition ************************/
void task_system_init(void *parameters)
{
	task_system_cfg_t *p_task_system_cfg;
	task_system_dta_t *p_task_system_dta;
	task_system_st_t	state;
	task_system_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_system_init), p_task_system);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_system), p_task_system_);

	g_task_system_cnt = G_TASK_SYS_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_system_cnt), g_task_system_cnt);

	init_queue_event_task_system();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_system_dta = &task_system_dta;
	p_task_system_cfg = &task_system_cfg;

	/* Print out: Task execution FSM */
	state = p_task_system_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_system_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_system_cfg->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	cycle_counter_init();
	cycle_counter_reset();

	g_task_system_tick_cnt = G_TASK_SYS_TICK_CNT_INI;
}

void task_system_update(void *parameters)
{
	task_system_cfg_t *p_task_system_cfg;
	task_system_dta_t *p_task_system_dta;
	//task_motor_dta_t *p_task_motor_dta;

	bool b_time_update_required = false;

	/* Update Task Menu Counter */
	g_task_system_cnt++;

	/* Protect shared resource (g_task_system_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_SYS_TICK_CNT_INI < g_task_system_tick_cnt)
    {
    	g_task_system_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_system_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_SYS_TICK_CNT_INI < g_task_system_tick_cnt)
		{
			g_task_system_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_system_cfg = &task_system_cfg;
		p_task_system_dta = &task_system_dta;

    	if (DEL_SYS_XX_MIN < p_task_system_cfg->tick)
		{
			p_task_system_cfg->tick--;
		}
		else
		{
			p_task_system_cfg->tick = DEL_SYS_XX_MAX;


			/* Implementacion maquina de estados */
			if (true == any_event_task_system())
			{
				p_task_system_cfg->flag = true;
				p_task_system_dta->event = get_event_task_system();
			}

			/* Revisamos eventos de baja prioridad */
			if ((true == p_task_system_cfg->flag) && (EV_SYS_ADC_REQ == p_task_system_dta->event))
			{
				p_task_system_dta->adc_req_pending = true;
				p_task_system_cfg->flag = false;
			}

			switch (p_task_system_dta->state)
			{
				case ST_SYS_IDLE:

					p_task_system_dta->tick_idle--;
					if (DEL_SYS_IDLE_MIN == p_task_system_dta->tick_idle)
					{
						if (p_task_system_cfg->mode_time)
						{
							//put_even_task_actuator(EV_ACT_RELAY_ON)
							p_task_system_dta->tick_riego = p_task_system_cfg->tick_riego_max;
							p_task_system_dta->state = ST_SYS_RIEGO;
						}
						else if (!(p_task_system_cfg->mode_time))
						{
							put_event_task_sht85(p_task_system_cfg->ev_sen_measure_on);
							p_task_system_dta->state = ST_SYS_MEASURE;
						}
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_CONFIG_ON == p_task_system_dta->event))
					{
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_CONFIG;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_RIEGO_ACT_ON == p_task_system_dta->event))
					{
						p_task_system_dta->tick_riego = p_task_system_cfg->tick_riego_max;
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_RIEGO;
					}
					else if (true == p_task_system_dta->adc_req_pending)
					{
						put_event_task_adc(p_task_system_cfg->ev_adc_start);
						p_task_system_dta->adc_req_pending = false;
						p_task_system_dta->state = ST_SYS_ADC_MEASURE;
					}
					break;

				case ST_SYS_MEASURE:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_READY_ON == p_task_system_dta->event))
					{
						put_event_task_sht85(p_task_system_cfg->ev_sen_measure_read);
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_WAITING;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_FALLA_ON == p_task_system_dta->event))
					{
						p_task_system_dta->tick_falla = p_task_system_cfg->tick_falla_max;
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_FALLA;
					}

					break;

				case ST_SYS_CONFIG:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_NCONFIG_ON == p_task_system_dta->event))
					{
						p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max;
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_IDLE;
					}

					break;

				case ST_SYS_RIEGO :

					p_task_system_dta->tick_riego--;
					if ((DEL_SYS_RIEGO_MIN == p_task_system_dta->tick_riego) || ((true == p_task_system_cfg->flag) && (EV_SYS_RIEGO_NACT_ON == p_task_system_dta->event)))
					{
						//put_even_task_actuator(EV_ACT_RELAY_OFF)
						p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max;
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_IDLE;
					}

					break;

				case ST_SYS_ADC_MEASURE:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_ADC_OK == p_task_system_dta->event))
					{
						get_values_task_adc(&p_task_system_dta->adc_temperature, &p_task_system_dta->adc_batery);

						if ((p_task_system_dta->adc_temperature > p_task_system_cfg->threshold_adc_temperature) || (p_task_system_dta->adc_batery < p_task_system_cfg->threshold_adc_batery))
						{
							p_task_system_dta->state = ST_SYS_FALLA;
						}
						put_event_task_menu( p_task_system_cfg->ev_men_adc_req_ok);
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_IDLE;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_ADC_NOT_OK == p_task_system_dta->event))
					{
						p_task_system_dta->tick_falla = p_task_system_cfg->tick_falla_max;
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_FALLA;
					}

					break;

				case ST_SYS_WAITING:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_CHECK_OK == p_task_system_dta->event))
					{

						get_values_task_sht85(&p_task_system_dta->temperature, &p_task_system_dta->humidity);

						if ((p_task_system_dta->temperature > p_task_system_cfg->threshold_temperature) && (p_task_system_dta->humidity < p_task_system_cfg->threshold_humidity))
						{
							//put_even_task_actuator(EV_ACT_RELAY_ON)
							p_task_system_dta->tick_riego = p_task_system_cfg->tick_riego_max;
							p_task_system_dta->state = ST_SYS_RIEGO;
						}
						else
						{
							//put_even_task_actuator(EV_ACT_RELAY_OFF)
							p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max;
							p_task_system_dta->state = ST_SYS_IDLE;
						}
						p_task_system_cfg->flag = false;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_CHECK_NOT_OK == p_task_system_dta->event))
					{
						p_task_system_dta->tick_falla = p_task_system_cfg->tick_falla_max;
						p_task_system_dta->state = ST_SYS_FALLA;
						p_task_system_cfg->flag = false;

					}

					break;

				case ST_SYS_FALLA:

					p_task_system_dta->tick_falla--;
					if (DEL_SYS_FALLA_MIN == p_task_system_dta->tick_falla)
					{
						put_event_task_sht85(p_task_system_cfg->ev_sen_falla_ok);
						p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max;
						p_task_system_dta->state = ST_SYS_IDLE;
					}

					break;

				default:

					p_task_system_cfg->tick  = DEL_SYS_XX_MIN;
					p_task_system_cfg->flag  = false;
					p_task_system_dta->state = ST_SYS_IDLE;
					p_task_system_dta->event = EV_SYS_RIEGO_NACT_ON;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/

