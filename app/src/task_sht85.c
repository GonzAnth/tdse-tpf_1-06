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

#define DEL_SEN_MEAS_XX_MAX			5ul
#define DEL_SEN_MEAS_XX_MIN			0ul

#define DEL_SEN_TIMEOUT_MAX			10ul
#define DEL_SEN_TIMEOUT_MIN			0ul

extern I2C_HandleTypeDef hi2c1;

/********************** internal data declaration ****************************/
task_sht85_cfg_t task_sht85_cfg = {
	DEL_SEN_XX_MIN, false, DEL_SEN_MEAS_XX_MAX, DEL_SEN_TIMEOUT_MAX,
};

task_sht85_dta_t task_sht85_dta = {
	.tick_measure 		= DEL_SEN_MEAS_XX_MIN,
	.tick_timeout		= DEL_SEN_TIMEOUT_MIN,
	.state				= ST_SEN_IDLE,
	.event				= EV_SEN_IDLE,
	.i2c_op_complete	= false,
	.i2c_error			= false
};


/********************** internal functions declaration ***********************/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	task_sht85_dta.i2c_op_complete = true;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    task_sht85_dta.i2c_op_complete = true;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    task_sht85_dta.i2c_error = true;
}

/********************** internal data definition *****************************/
const char *p_task_sht85 	= "Task SHT85";
const char *p_task_sht85_ 	= "Non-Blocking & Update By Time Code";

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
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_sht85_init), p_task_sht85);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_sht85), p_task_sht85_);

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


			/* Implementacion maquina de estados */
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
						p_task_sht85_dta->i2c_op_complete = false;
						p_task_sht85_dta->i2c_error	= false;
						if (true == SHT85_start_measure_IT())
						{
							p_task_sht85_dta->tick_timeout = p_task_sht85_cfg->tick_timeout_max;
							p_task_sht85_dta->state = ST_SEN_WAIT_TX;
						}
						else
						{
							put_event_task_system(EV_SYS_FALLA);
							p_task_sht85_dta->state = ST_SEN_FALLA;
						}
					}

					break;


				case ST_SEN_WAIT_TX:
					if (true == p_task_sht85_dta->i2c_error)
					{
						put_event_task_system(EV_SYS_FALLA);
						p_task_sht85_dta->state = ST_SEN_FALLA;
					}
					else if (true == p_task_sht85_dta->i2c_op_complete)
					{
						p_task_sht85_dta->tick_measure = p_task_sht85_cfg->tick_means_max;
						p_task_sht85_dta->state = ST_SEN_WAITING;
					}
					else
					{
						p_task_sht85_dta->tick_timeout--;
						if (DEL_SEN_TIMEOUT_MIN == p_task_sht85_dta->tick_timeout)
						{
							put_event_task_system(EV_SYS_FALLA);
							p_task_sht85_dta->state = ST_SEN_FALLA;
						}
					}

					break;



				case ST_SEN_WAITING:
					p_task_sht85_dta->tick_measure--;
					if (DEL_SEN_MEAS_XX_MIN == p_task_sht85_dta->tick_measure)
					{
						put_event_task_system(EV_SYS_READY);
						p_task_sht85_dta->state = ST_SEN_READY;
					}

					break;


				case ST_SEN_READY:
					if ((true == p_task_sht85_cfg->flag) && (EV_SEN_MEASURE_READ == p_task_sht85_dta->event))
					{
						p_task_sht85_dta->i2c_op_complete = false;
						p_task_sht85_dta->i2c_error = false;
						if (true == SHT85_start_read_IT(p_task_sht85_dta->i2c_rx_raw_values))
						{
							p_task_sht85_dta->tick_timeout = p_task_sht85_cfg->tick_timeout_max;
							p_task_sht85_dta->state = ST_SEN_WAIT_RX;
						}
						else
						{
							put_event_task_system(EV_SYS_CHECK_NOT_OK);
							p_task_sht85_dta->state = ST_SEN_FALLA;
						}
						p_task_sht85_cfg->flag = false;
					}

					break;


				case ST_SEN_WAIT_RX:
					if (true == p_task_sht85_dta->i2c_error)
					{
						put_event_task_system(EV_SYS_FALLA);
						p_task_sht85_dta->state = ST_SEN_FALLA;
					} else if (true == p_task_sht85_dta->i2c_op_complete)
					{
						bool calculo = SHT85_compute_values(p_task_sht85_dta->i2c_rx_raw_values,
														&p_task_sht85_dta->temperature,
														&p_task_sht85_dta->humidity);

						if (true == calculo)
						{
							put_event_task_system(EV_SYS_CHECK_OK);
							p_task_sht85_dta->state = ST_SEN_IDLE;
						} else {
							put_event_task_system(EV_SYS_CHECK_NOT_OK);
							p_task_sht85_dta->state = ST_SEN_FALLA;
						}
					}
					else
					{
						p_task_sht85_dta->tick_timeout--;
						if (DEL_SEN_TIMEOUT_MIN == p_task_sht85_dta->tick_timeout)
						{
							put_event_task_system(EV_SYS_CHECK_NOT_OK);
							p_task_sht85_dta->state = ST_SEN_FALLA;
						}

					}


					break;


				case ST_SEN_FALLA:
					if ((true == p_task_sht85_cfg->flag) && (EV_SEN_FALLA_OK == p_task_sht85_dta->event))
					{
						/* Reset I2C */
						HAL_I2C_Master_Abort_IT(&hi2c1, SHT85_I2C_ADDR);
						HAL_I2C_DeInit(&hi2c1);
						HAL_I2C_Init(&hi2c1);

						p_task_sht85_dta->i2c_op_complete = false;
						p_task_sht85_dta->i2c_error = false;
						p_task_sht85_cfg->flag = false;
						p_task_sht85_dta->state = ST_SEN_IDLE;
					}

					break;


				default:
					p_task_sht85_cfg->tick  = DEL_SEN_XX_MIN;
					p_task_sht85_dta->state = ST_SEN_IDLE;
					p_task_sht85_dta->event = EV_SEN_IDLE;
					p_task_sht85_cfg->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/
