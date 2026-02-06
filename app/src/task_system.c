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
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
#include "flash.h"

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI			0ul
#define G_TASK_SYS_TICK_CNT_INI		0ul

#define DEL_SYS_XX_MIN				0ul
#define DEL_SYS_XX_MED				5ul
#define DEL_SYS_XX_MAX				500ul

#define SYS_TICK_TO_MIN				(60 * 1000 / DEL_SYS_XX_MAX)
#define SYS_TICK_TO_SEC				(1000 / DEL_SYS_XX_MAX)
#define SYS_DEBUG_MODE()			(HAL_GPIO_ReadPin(DEBUG_PIN_GPIO_Port, DEBUG_PIN_Pin) == GPIO_PIN_SET)

#define DEL_SYS_IDLE_MAX			60ul
#define DEL_SYS_IDLE_MIN			0ul

#define DEL_SYS_RIEGO_MAX			20ul
#define DEL_SYS_RIEGO_MIN			0ul

#define DEL_SYS_FALLA_MAX			10ul
#define DEL_SYS_FALLA_MIN			0ul

#define THRESHOLD_SYS_TEMP_DEF		25ul
#define THRESHOLD_SYS_HUM_DEF		80ul

#define THRESHOLD_SYS_ADC_TEMP_DEF		80ul
#define THRESHOLD_SYS_ADC_BAT_DEF		1ul

/********************** internal data declaration ****************************/
task_system_cfg_t task_system_cfg = {
	DEL_SYS_XX_MIN, false,
	DEL_SYS_IDLE_MAX, DEL_SYS_RIEGO_MAX, DEL_SYS_FALLA_MAX,
	SYS_MOD_MANUAL, THRESHOLD_SYS_TEMP_DEF, THRESHOLD_SYS_HUM_DEF, THRESHOLD_SYS_ADC_TEMP_DEF, THRESHOLD_SYS_ADC_BAT_DEF,
};

task_system_dta_t task_system_dta = {
	.tick_idle 			= 	0,
	.tick_riego 		= 	0,
	.tick_falla			=	DEL_SYS_FALLA_MIN,
	.state				=	ST_SYS_IDLE,
	.last_state			=	ST_SYS_IDLE,
	.event				=	EV_SYS_IDLE,

	.adc_req_pending	=	false,
};

#define SYSTEM_DTA_QTY	(sizeof(task_system_dta)/sizeof(task_system_dta_t))

/********************** internal functions declaration ***********************/

static uint32_t get_scaled_tick(uint32_t tick);

uint32_t get_system_remaining_time(void);


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


	/* Lectura de FLASH */
	flash_setup_t stored_config;
	Flash_Read_Setup(&stored_config);
	if (stored_config.magic_number == FLASH_MAGIC_NUMBER) {
		task_system_cfg.tick_idle_max         = stored_config.tick_idle_max;
		task_system_cfg.tick_riego_max        = stored_config.tick_riego_max;
		task_system_cfg.threshold_temperature = stored_config.threshold_temperature;
		task_system_cfg.threshold_humidity    = stored_config.threshold_humidity;
		LOGGER_LOG("   Flash loaded OK\r\n");
	} else {
		flash_setup_t default_config = {
			.magic_number = FLASH_MAGIC_NUMBER,
			.tick_idle_max = DEL_SYS_IDLE_MAX,
			.tick_riego_max = DEL_SYS_RIEGO_MAX,
			.threshold_temperature = THRESHOLD_SYS_TEMP_DEF,
			.threshold_humidity = THRESHOLD_SYS_HUM_DEF
		};
		Flash_Write_Setup(&default_config);
		LOGGER_LOG("   Flash Init with default config\r\n");
	}

	p_task_system_dta->tick_idle = get_scaled_tick(p_task_system_cfg->tick_idle_max);
	p_task_system_dta->tick_riego = get_scaled_tick(p_task_system_cfg->tick_riego_max);


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


			/* Aquí colocamos código a ejecutar cuando cambiamos de estado */
			if (p_task_system_dta->state != p_task_system_dta->last_state)
			{

				/* AVISOS EN CASO DE FALLA*/
				if (p_task_system_dta->state == ST_SYS_FALLA)
				{
					put_event_task_menu(EV_MEN_SYS_FALLA);
					put_event_task_actuator(EV_ACT_ON, ID_ACT_BUZZER);
				}

				p_task_system_dta->last_state = p_task_system_dta->state;
			}



			/* Implementacion maquina de estados */
			if (true == any_event_task_system())
			{
				p_task_system_cfg->flag = true;
				p_task_system_dta->event = get_event_task_system();

				/* Revisamos eventos de baja prioridad */
				if (EV_SYS_ADC_REQ == p_task_system_dta->event)
				{
					p_task_system_dta->adc_req_pending = true;
					p_task_system_cfg->flag = false;
				}
				else if(EV_SYS_MOD_MANUAL == p_task_system_dta->event)
				{
					p_task_system_cfg->system_mode = SYS_MOD_MANUAL;
					p_task_system_cfg->flag = false;
				}
				else if(EV_SYS_MOD_TIME == p_task_system_dta->event)
				{
					p_task_system_cfg->system_mode = SYS_MOD_TIME;
					p_task_system_cfg->flag = false;
				}
				else if(EV_SYS_MOD_SENSOR == p_task_system_dta->event)
				{
					p_task_system_cfg->system_mode = SYS_MOD_SENSOR;
					p_task_system_cfg->flag = false;
				}
			}

			switch (p_task_system_dta->state)
			{
				case ST_SYS_IDLE:

					p_task_system_dta->tick_idle--;
					if (DEL_SYS_IDLE_MIN == p_task_system_dta->tick_idle)
					{
						if (SYS_MOD_TIME == p_task_system_cfg->system_mode)
						{

							put_event_task_actuator(EV_ACT_ON, ID_ACT_RELAY);
							p_task_system_dta->tick_riego = get_scaled_tick(p_task_system_cfg->tick_riego_max);
							p_task_system_dta->state = ST_SYS_RIEGO;
						}
						else if (SYS_MOD_SENSOR == p_task_system_cfg->system_mode)
						{
							put_event_task_sht85(EV_SEN_MEASURE_ON);
							p_task_system_dta->state = ST_SYS_MEASURE;
						}
						else if (SYS_MOD_MANUAL == p_task_system_cfg->system_mode)
						{
							p_task_system_dta->tick_idle = get_scaled_tick(p_task_system_cfg->tick_idle_max);
						}
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_CONFIG_ON == p_task_system_dta->event))
					{
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_CONFIG;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_RIEGO_ON == p_task_system_dta->event))
					{
						put_event_task_actuator(EV_ACT_ON, ID_ACT_RELAY);
						p_task_system_dta->tick_riego = get_scaled_tick(p_task_system_cfg->tick_riego_max);
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_RIEGO;
					}
					else if (true == p_task_system_dta->adc_req_pending)
					{
						put_event_task_adc(EV_ADC_START);
						p_task_system_dta->adc_req_pending = false;
						p_task_system_dta->state = ST_SYS_ADC_MEASURE;
					}
					break;

				case ST_SYS_MEASURE:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_READY == p_task_system_dta->event))
					{
						put_event_task_sht85(EV_SEN_MEASURE_READ);
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_WAITING;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_FALLA == p_task_system_dta->event))
					{
						p_task_system_dta->tick_falla = p_task_system_cfg->tick_falla_max;
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_FALLA;
					}

					break;


				case ST_SYS_CONFIG:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_CONFIG_NEW == p_task_system_dta->event))
					{
						flash_setup_t to_save = {
							.magic_number = FLASH_MAGIC_NUMBER,
							.tick_idle_max = p_task_system_cfg->tick_idle_max,
							.tick_riego_max = p_task_system_cfg->tick_riego_max,
							.threshold_temperature = p_task_system_cfg->threshold_temperature,
							.threshold_humidity = p_task_system_cfg->threshold_humidity,
						};
						Flash_Write_Setup(&to_save);

						p_task_system_dta->tick_idle = get_scaled_tick(p_task_system_cfg->tick_idle_max);
						p_task_system_dta->tick_riego = get_scaled_tick(p_task_system_cfg->tick_riego_max);
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_IDLE;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_CONFIG_OFF == p_task_system_dta->event))
					{
						p_task_system_cfg->flag = false;
						p_task_system_dta->state = ST_SYS_IDLE;
					}

					break;


				case ST_SYS_RIEGO :

					p_task_system_dta->tick_riego--;
					if ((DEL_SYS_RIEGO_MIN == p_task_system_dta->tick_riego) || ((true == p_task_system_cfg->flag) && (EV_SYS_RIEGO_OFF == p_task_system_dta->event)))
					{
						put_event_task_actuator(EV_ACT_OFF, ID_ACT_RELAY);
						p_task_system_dta->tick_idle = get_scaled_tick(p_task_system_cfg->tick_idle_max);
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
						put_event_task_menu(EV_MEN_ADC_REQ_OK);
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
							put_event_task_actuator(EV_ACT_ON, ID_ACT_RELAY);
							p_task_system_dta->tick_riego = get_scaled_tick(p_task_system_cfg->tick_riego_max);
							p_task_system_dta->state = ST_SYS_RIEGO;
						}
						else
						{
							//put_even_task_actuator(EV_ACT_RELAY_OFF)
							p_task_system_dta->tick_idle = get_scaled_tick(p_task_system_cfg->tick_idle_max);
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
						put_event_task_sht85(EV_SEN_FALLA_OK);
						put_event_task_adc(EV_ADC_FALLA_OK);
						put_event_task_actuator(EV_ACT_OFF, ID_ACT_BUZZER);
						p_task_system_dta->tick_idle = get_scaled_tick(p_task_system_cfg->tick_idle_max);
						p_task_system_dta->state = ST_SYS_IDLE;
					}

					break;

				default:

					p_task_system_cfg->tick  = DEL_SYS_XX_MIN;
					p_task_system_cfg->flag  = false;
					p_task_system_dta->state = ST_SYS_IDLE;
					p_task_system_dta->event = EV_SYS_RIEGO_OFF;

					break;
			}
		}
	}
}


static uint32_t get_scaled_tick(uint32_t tick)
{
	if (SYS_DEBUG_MODE())
	{
		return tick * SYS_TICK_TO_SEC;
	} else {
		return tick * SYS_TICK_TO_MIN;
	}
}

uint32_t get_system_remaining_time(void)
{
    uint32_t ticks = 0;
    if (task_system_dta.state == ST_SYS_RIEGO)
    {
        ticks = task_system_dta.tick_riego;
    } else if (task_system_dta.state == ST_SYS_IDLE)
    {
        ticks = task_system_dta.tick_idle;
    }
	return ticks / SYS_TICK_TO_MIN;
}

/********************** end of file ******************************************/

