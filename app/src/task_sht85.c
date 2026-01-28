/**
 * @file    task_sht85.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Tarea asociada a la máquina de estados del sensor SHT85.
 * @date    Dic 15, 2025
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
#include "sht85.h"
#include "task_sht85_attribute.h"
#include "task_sht85_interface.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"

/********************** macros and definitions *******************************/
#define G_TASK_SEN_CNT_INI			0ul
#define G_TASK_SEN_TICK_CNT_INI		0ul

#define DEL_SEN_XX_MIN				0ul
#define DEL_SEN_XX_MED				50ul
#define DEL_SEN_XX_MAX				500ul

#define DEL_SEN_MEAS_XX_MAX			20ul
#define DEL_SEN_MEAS_XX_MIN			0ul

extern I2C_HandleTypeDef hi2c1;

/********************** internal data declaration ****************************/
task_sht85_cfg_t task_sht85_cfg = {
	DEL_SEN_XX_MIN, false, DEL_SEN_MEAS_XX_MAX,
	EV_SYS_FALLA_ON, EV_SYS_READY_ON, EV_SYS_CHECK_OK, EV_SYS_CHECK_NOT_OK
};

task_sht85_dta_t task_sht85_dta = {
	DEL_SEN_MEAS_XX_MIN	, ST_SEN_IDLE, EV_SEN_MEASURE_OFF, true, false, 0, 0, 0
};

#define MENU_DTA_QTY	(sizeof(task_sen_dta)/sizeof(task_sen_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_sen 		= "Task Menu (Interactive Menu)";
const char *p_task_sen_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_sht85_cnt;
volatile uint32_t g_task_sht85_tick_cnt;

/********************** external functions definition ************************/
void task_sht85_init(void *parameters)
{
	task_sht85_cfg_t *p_task_sht85_cfg;
	task_sht85_dta_t *p_task_sht85_dta;
	task_sht85_st_t	state;
	task_sht85_ev_t	event;
	bool b_event;

	/* Inicializamos el driver de hardware con el handle del I2C */
	SHT85_Init(&hi2c1);
	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_sen_init), p_task_sen);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_sen), p_task_sen_);

	g_task_sht85_cnt = G_TASK_SEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_sht85_cnt), g_task_sht85_cnt);

	init_queue_event_task_sht85();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_sht85_dta = &task_sht85_dta;
	p_task_sht85_cfg = &task_sht85_cfg;

	/* Print out: Task execution FSM */
	state = p_task_sht85_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_sht85_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_sht85_cfg->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	cycle_counter_init();
	cycle_counter_reset();

	g_task_sht85_tick_cnt = G_TASK_SEN_TICK_CNT_INI;
}

void task_sht85_update(void *parameters)
{
	task_sht85_cfg_t *p_task_sht85_cfg;
	task_sht85_dta_t *p_task_sht85_dta;
	//task_motor_dta_t *p_task_motor_dta;

	bool b_time_update_required = false;

	/* Update Task Menu Counter */
	g_task_sht85_cnt++;

	/* Protect shared resource (g_task_sen_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_SEN_TICK_CNT_INI < g_task_sht85_tick_cnt)
    {
    	g_task_sht85_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_sen_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_SEN_TICK_CNT_INI < g_task_sht85_tick_cnt)
		{
			g_task_sht85_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_sht85_cfg = &task_sht85_cfg;
		p_task_sht85_dta = &task_sht85_dta;

    	if (DEL_SEN_XX_MIN < p_task_sht85_cfg->tick)
		{
			p_task_sht85_cfg->tick--;
		}
		else
		{
			p_task_sht85_cfg->tick = DEL_SEN_XX_MAX;

			 /* Aquí colocamos código a ejecutar cuando cambiamos de estado

			if (p_task_sen_dta->state != p_task_sen_dta->last_state)
			{
			    p_task_sen_dta->refresh_screen = true;
			    p_task_sen_dta->last_state = p_task_sen_dta->state;
			}

			 Implementacion maquina de estados */
			if (true == any_event_task_sht85())
			{
				p_task_sht85_cfg->flag = true;
				p_task_sht85_dta->event = get_event_task_sht85();
			}

			switch (p_task_sht85_dta->state)
			{
				case ST_SEN_IDLE:

					if ((true == p_task_sht85_cfg->flag) && (EV_SEN_MEASURE_ON == p_task_sht85_dta->event))
					{
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_MEASURE;
					}

					break;


				case ST_SEN_MEASURE:

					p_task_sht85_dta->ready= SHT85_send_single_shot();

					if (true == p_task_sht85_dta->ready)
					{
						p_task_sht85_dta->tick_means = p_task_sht85_cfg->tick_means_max;
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_WAITING;
					}
					else
					{
						put_event_task_system(p_task_sht85_cfg->ev_sys_falla);
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_FALLA;
					}
					/* VERIFICACION CON EVENTOS
					if ((true == p_task_sht85_cfg->flag) && (EV_SEN_MEASURE_OK == p_task_sht85_dta->event))
					{
						p_task_sht85_dta->tick_means = p_task_sht85_cfg->tick_means_max;
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_WAITING;
					}
					else if ((true == p_task_sht85_cfg->flag) && (EV_SEN_MEASURE_NOT_OK  == p_task_sht85_dta->event))
					{
						put_event_task_system(p_task_sht85_cfg->ev_sys_falla);
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_FALLA;
					}*/

					break;

				case ST_SEN_WAITING:

					p_task_sht85_dta->tick_means--;
					if (DEL_SEN_MEAS_XX_MIN == p_task_sht85_dta->tick_means)
					{
						put_event_task_system(p_task_sht85_cfg->ev_sys_ready_on);
						p_task_sht85_dta->state = ST_SEN_READY;
					}

					break;

				case ST_SEN_READY:

					if ((true == p_task_sht85_cfg->flag) && (EV_SEN_MEASURE_READ == p_task_sht85_dta->event))
					{
						p_task_sht85_dta->measure_check = SHT85_read(&p_task_sht85_dta->temperature, &p_task_sht85_dta->humidity);

						if (p_task_sht85_dta->measure_check == true)
						{
							put_event_task_system(p_task_sht85_cfg->ev_sys_check_ok);
							p_task_sht85_dta->state = ST_SEN_IDLE;
						}
						else
						{
							put_event_task_system(p_task_sht85_cfg->ev_sys_check_not_ok);
							p_task_sht85_dta->state = ST_SEN_FALLA;
						}

						p_task_sht85_cfg->flag = false;
					}

					break;

				// esto es otra opcion por si hace falta definir un estado de check
				/*case ST_SEN_CHECK:

					p_task_sht85_dta->measure_check = false; // funcion para verificar la medicion
					if (p_task_sht85_dta->measure_check == true)
					{
						p_task_sht85_dta->state = ST_SEN_IDLE;
					}
					else
					{
						p_task_sht85_dta->state = ST_SEN_FALLA;
					}

					break;*/

				case ST_SEN_FALLA:

					if ((true == p_task_sht85_cfg->flag) && (EV_SEN_FALLA_OK == p_task_sht85_dta->event))
					{
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_IDLE;
					}

					break;

				default:

					p_task_sht85_cfg->tick  = DEL_SEN_XX_MIN;
					p_task_sht85_dta->state = ST_SEN_IDLE;
					p_task_sht85_dta->event = EV_SEN_MEASURE_OFF;
					p_task_sht85_cfg->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/
