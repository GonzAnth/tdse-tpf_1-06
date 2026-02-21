/**
 * @file    task_actuator.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Tarea asociada a la máquina de estados de los actuadores.
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
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"

/********************** macros and definitions *******************************/
#define G_TASK_ACT_CNT_INIT			0ul
#define G_TASK_ACT_TICK_CNT_INI		0ul

#define DEL_ACT_XX_PUL				8ul
#define DEL_ACT_XX_BLI				500ul
#define DEL_ACT_XX_MIN				0ul

/********************** internal data declaration ****************************/
const task_actuator_cfg_t task_actuator_cfg_list[] = {
	{ID_ACT_RELAY,  RELAY_PORT,  RELAY_PIN, RELAY_ON,  RELAY_OFF, DEL_ACT_XX_PUL},
	{ID_ACT_BUZZER, BUZZER_PORT, BUZZER_PIN, BUZZER_ON, BUZZER_OFF, DEL_ACT_XX_PUL}
};

#define ACTUATOR_CFG_QTY	(sizeof(task_actuator_cfg_list)/sizeof(task_actuator_cfg_t))

task_actuator_dta_t task_actuator_dta_list[] = {
	{DEL_ACT_XX_MIN, DEL_ACT_XX_MIN, ST_ACT_OFF, EV_ACT_IDLE, false},
	{DEL_ACT_XX_MIN, DEL_ACT_XX_MIN, ST_ACT_OFF, EV_ACT_IDLE, false}
};

#define ACTUATOR_DTA_QTY	(sizeof(task_actuator_dta_list)/sizeof(task_actuator_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_actuator 		= "Task Actuator";
const char *p_task_actuator_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_actuator_cnt;
volatile uint32_t g_task_actuator_tick_cnt;

/********************** external functions definition ************************/
void task_actuator_init(void *parameters)
{
	uint32_t index;
	const task_actuator_cfg_t *p_task_actuator_cfg;
	task_actuator_dta_t *p_task_actuator_dta;
	task_actuator_st_t state;
	task_actuator_ev_t event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_actuator_init), p_task_actuator);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_actuator), p_task_actuator_);

	g_task_actuator_cnt = G_TASK_ACT_CNT_INIT;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_actuator_cnt), g_task_actuator_cnt);

	for (index = 0; ACTUATOR_DTA_QTY > index; index++)
	{
		/* Update Task Actuator Configuration & Data Pointer */
		p_task_actuator_cfg = &task_actuator_cfg_list[index];
		p_task_actuator_dta = &task_actuator_dta_list[index];

		/* Print out: Index & Task execution FSM */
		LOGGER_LOG("   %s = %lu", GET_NAME(index), index);

		state = p_task_actuator_dta->state;
		LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

		event = p_task_actuator_dta->event;
		LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

		b_event = p_task_actuator_dta->flag;
		LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

		HAL_GPIO_WritePin(p_task_actuator_cfg->gpio_port, p_task_actuator_cfg->pin, p_task_actuator_cfg->act_off); // apago todos lo actuadores
	}

	g_task_actuator_tick_cnt = G_TASK_ACT_TICK_CNT_INI;
}

void task_actuator_update(void *parameters)
{
	uint32_t index;
	const task_actuator_cfg_t *p_task_actuator_cfg;
	task_actuator_dta_t *p_task_actuator_dta;
	bool b_time_update_required = false;

	/* Update Task Actuator Counter */
	g_task_actuator_cnt++;

	/* Protect shared resource (g_task_actuator_tick_cnt) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_ACT_TICK_CNT_INI < g_task_actuator_tick_cnt)
    {
    	g_task_actuator_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_actuator_tick_cnt) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_ACT_TICK_CNT_INI < g_task_actuator_tick_cnt)
		{
			g_task_actuator_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	for (index = 0; ACTUATOR_DTA_QTY > index; index++)
		{
    		/* Update Task Actuator Configuration & Data Pointer */
			p_task_actuator_cfg = &task_actuator_cfg_list[index];
			p_task_actuator_dta = &task_actuator_dta_list[index];

			switch (p_task_actuator_dta->state)
			{
				case ST_ACT_OFF:

					if ((true == p_task_actuator_dta->flag) && (EV_ACT_ON == p_task_actuator_dta->event))
					{
						p_task_actuator_dta->flag = false;
						HAL_GPIO_WritePin(p_task_actuator_cfg->gpio_port, p_task_actuator_cfg->pin, p_task_actuator_cfg->act_on);
						p_task_actuator_dta->state = ST_ACT_ON;
					}
					else if ((true == p_task_actuator_dta->flag) && (EV_ACT_PULSE == p_task_actuator_dta->event))
					{
						p_task_actuator_dta->flag = false;
						HAL_GPIO_WritePin(p_task_actuator_cfg->gpio_port, p_task_actuator_cfg->pin, p_task_actuator_cfg->act_on);

						// Cargamos tiempo del pulso
						p_task_actuator_dta->tick_pulse = p_task_actuator_cfg->tick_pulse_max;
						p_task_actuator_dta->state = ST_ACT_PULSE;
					}

					break;


				case ST_ACT_ON:
					if ((true == p_task_actuator_dta->flag) && (EV_ACT_OFF == p_task_actuator_dta->event))
					{
						p_task_actuator_dta->flag = false;
						HAL_GPIO_WritePin(p_task_actuator_cfg->gpio_port, p_task_actuator_cfg->pin, p_task_actuator_cfg->act_off);
						p_task_actuator_dta->state = ST_ACT_OFF;
					}

					break;


				case ST_ACT_PULSE:
					p_task_actuator_dta->tick_pulse--;
					if ((DEL_ACT_XX_MIN == p_task_actuator_dta->tick_pulse))
					{
						HAL_GPIO_WritePin(p_task_actuator_cfg->gpio_port, p_task_actuator_cfg->pin, p_task_actuator_cfg->act_off);
						p_task_actuator_dta->state = ST_ACT_OFF;
					}

					break;



				default:
					p_task_actuator_dta->state = ST_ACT_OFF;
					p_task_actuator_dta->flag = false;

					break;
			}
		}
    }
}

/********************** end of file ******************************************/


