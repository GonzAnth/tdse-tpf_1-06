/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
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
#include "task_system_attribute.h"
#include "task_system_interface.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul

#define DEL_SYS_IDLE_MAX			30ul
#define DEL_SYS_IDLE_MIN			0ul

#define DEL_SYS_RIEGO_MAX			10ul
#define DEL_SYS_RIEGO_MIN			0ul

#define DEL_SYS_FALLA_MAX			5ul
#define DEL_SYS_FALLA_MIN			0ul

/********************** internal data declaration ****************************/
task_system_cfg_t task_system_cfg =
	{DEL_MEN_XX_MIN, false, DEL_SYS_IDLE_MAX, DEL_SYS_RIEGO_MAX, DEL_SYS_FALLA_MAX, 0, 0};

task_system_dta_t task_system_dta = {
	DEL_SYS_IDLE_MAX, DEL_SYS_RIEGO_MIN,DEL_SYS_FALLA_MIN, ST_SYS_IDLE, EV_SYS_RIEGO_NACT_ON, false, true, false, 0 , 0
};

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

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
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_system);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_system_);

	g_task_system_cnt = G_TASK_MEN_CNT_INI;

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

	g_task_system_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_system_update(void *parameters)
{
	task_system_cfg_t *p_task_system_cfg;
	task_system_dta_t *p_task_system_dta;
	//task_motor_dta_t *p_task_motor_dta;

	bool b_time_update_required = false;

	/* Update Task Menu Counter */
	g_task_system_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_system_tick_cnt)
    {
    	g_task_system_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_system_tick_cnt)
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

    	if (DEL_MEN_XX_MIN < p_task_system_cfg->tick)
		{
			p_task_system_cfg->tick--;
		}
		else
		{
			p_task_system_cfg->tick = DEL_MEN_XX_MAX;

			 /* Aquí colocamos código a ejecutar cuando cambiamos de estado

			if (p_task_menu_dta->state != p_task_menu_dta->last_state)
			{
			    p_task_menu_dta->refresh_screen = true;
			    p_task_menu_dta->last_state = p_task_menu_dta->state;
			}

			 Implementacion maquina de estados */
			if (true == any_event_task_system())
			{
				p_task_system_cfg->flag = true;
				p_task_system_dta->event = get_event_task_system();
			}

			switch (p_task_system_dta->state)
			{
				case ST_SYS_IDLE:

					p_task_system_dta->tick_idle--;
					if (DEL_SYS_IDLE_MIN == p_task_system_dta->tick_idle)
					{
						if (true == p_task_system_dta->mod_time)
						{
							//put_even_task_actuator(EV_ACT_RELAY_ON)
							p_task_system_dta->tick_riego = p_task_system_cfg->tick_riego_max;
							p_task_system_dta->state = ST_SYS_RIEGO;
						}
						else if (true == p_task_system_dta->mod_sensor)
						{
							//put_even_task_sensor(EV_SEN_MEASURE_ON)
							p_task_system_dta->state = ST_SYS_MEASURE;
						}
					}
					/* else if (true == p_task_system_dta->mod_config) // ESTO QUE SEA UN EVENTO
					{
						p_task_system_dta->state = ST_SYS_CONFIG;
					}*/
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
					break;


				case ST_SYS_MEASURE:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_READY_ON == p_task_system_dta->event))
					{
						//put_even_task_sensor(EV_SEN_MEASURE_READ)
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

					/*if (true == p_task_system_dta->mod_config)
					{
						// actualizar todos los parametris de tiempo y umbrales de temperatura
						p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max; // este seria el nuevo tiempo maximo
						p_task_system_dta->state = ST_SYS_IDLE;
					}*/
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

				case ST_SYS_WAITING:

					if ((true == p_task_system_cfg->flag) && (EV_SYS_CHECK_OK == p_task_system_dta->event))
					{
						// funciones para extraer la tempre y humedad de la ME del sensor
						p_task_system_dta->temperature = 14;
						p_task_system_dta->humidity = 4;

						if ((p_task_system_dta->temperature > 20) && (p_task_system_dta->temperature > 5))
						{
						//put_even_task_actuator(EV_ACT_RELAY_ON)
							p_task_system_dta->tick_riego = p_task_system_cfg->tick_riego_max;
							p_task_system_dta->state = ST_SYS_RIEGO;
						}
						else
						{
							//put_even_task_actuator(EV_ACT_RELAY_ON)
							p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max;
							p_task_system_dta->state = ST_SYS_IDLE;
						}
						p_task_system_cfg->flag = false;
					}
					else if ((true == p_task_system_cfg->flag) && (EV_SYS_CHECK_NOT_OK == p_task_system_dta->event))
					{
						//put_even_task_actuator(EV_ACT_RELAY_OFF)
						p_task_system_dta->tick_falla = p_task_system_cfg->tick_falla_max;
						p_task_system_dta->state = ST_SYS_FALLA;
						p_task_system_cfg->flag = false;

					}

					break;

				case ST_SYS_FALLA:

					p_task_system_dta->tick_falla--;
					if (DEL_SYS_FALLA_MIN == p_task_system_dta->tick_falla)
					{
						//put_even_task_sensor(EV_SEN_FALLA_OK)
						p_task_system_dta->tick_idle = p_task_system_cfg->tick_idle_max;
						p_task_system_dta->state = ST_SYS_IDLE;
					}

					break;

				default:

					p_task_system_cfg->tick  = DEL_MEN_XX_MIN;
					p_task_system_cfg->flag  = false;
					p_task_system_dta->state = ST_SYS_IDLE;
					p_task_system_dta->event = EV_SYS_RIEGO_NACT_ON;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/
