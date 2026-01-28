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
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				5ul
#define DEL_MEN_XX_MAX				50ul

#define DEL_MEN_IDLE_MIN			0ul
#define DEL_MEN_IDLE_MAX			40ul

#define THRESHOLD_MEN_TEMP_DEF		24ul
#define THRESHOLD_MEN_HUM_DEF		30ul

/********************** internal data declaration ****************************/
task_menu_cfg_t task_menu_cfg = {
	DEL_MEN_XX_MIN, false, false, DEL_MEN_IDLE_MAX,
	EV_SYS_CONFIG_ON, EV_SYS_NCONFIG_ON, EV_SYS_RIEGO_ACT_ON, EV_SYS_RIEGO_NACT_ON, EV_SYS_ADC_REQ,
};

task_menu_dta_t task_menu_dta = {
	DEL_MEN_IDLE_MIN, ST_MEN_MAIN, ST_MEN_MAIN, EV_MEN_ENT_IDLE,
	true, THRESHOLD_MEN_TEMP_DEF, THRESHOLD_MEN_TEMP_DEF
};

/*task_aspersor_dta_t task_aspersor_dta = {
	false,TIME_MEN_RIEGO_MIN, TEMP_MEN_RIEGO_MIN, HUME_MEN_RIEGO_MIN, false, false
};*/

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	task_menu_cfg_t *p_task_menu_cfg;
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_menu_);

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_cfg = &task_menu_cfg;
	p_task_menu_dta = &task_menu_dta;

	/* Print out: Task execution FSM */
	state = p_task_menu_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_menu_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_menu_cfg->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	cycle_counter_init();
	cycle_counter_reset();

	displayInit( DISPLAY_CONNECTION_GPIO_4BITS );

	displayCharPositionWrite(1, 3);
	displayStringWrite("ENTER / NEXT / ESC");

	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	task_menu_cfg_t *p_task_menu_cfg;
	task_menu_dta_t *p_task_menu_dta;

	bool b_time_update_required = false;
	char str_buffer[ANCHO_LCD + 1]; //se suma caracter \0

	/* Update Task Menu Counter */
	g_task_menu_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_menu_cfg = &task_menu_cfg;
		p_task_menu_dta = &task_menu_dta;

    	if (DEL_MEN_XX_MIN < p_task_menu_cfg->tick)
		{
			p_task_menu_cfg->tick--;
		}
		else
		{
			p_task_menu_cfg->tick = DEL_MEN_XX_MAX;

			 /* Aquí colocamos código a ejecutar cuando cambiamos de estado */
			if (p_task_menu_dta->state != p_task_menu_dta->last_state)
			{
			    p_task_menu_cfg->refresh_screen = true;
			    p_task_menu_dta->last_state = p_task_menu_dta->state;
			}

			/* Implementacion maquina de estados */
			if (true == any_event_task_menu())
			{
				p_task_menu_cfg->flag = true;
				p_task_menu_dta->event = get_event_task_menu();
			}

			switch (p_task_menu_dta->state)
			{
				case ST_MEN_MAIN:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 5, "MENU MAIN");
						displayUpdateRow(1, 0, "ENTER PARA CONTINUAR");
						displayClearRow(2);
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_MANUAL;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_adc_req);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_SALUD_WAIT;
					}

					break;

				case ST_MEN_SALUD_WAIT:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 5, "ESTADO");
						displayUpdateRow(1, 0, "ESC PARA VOLVER");
						displayClearRow(2);
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ADC_REQ_OK == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_SALUD_SHOW;
					}

					break;

				case ST_MEN_SALUD_SHOW:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 5, "El estado del sitema es ");
						displayUpdateRow(1, 0, "ESC PARA VOLVER");
						displayClearRow(2);
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;


				case ST_MEN_MODE_MANUAL:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 0, "SELECCIONAR MODO:");
						displayUpdateRow(1, 0, "MODO MANUAL");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_RIEGO_OFF;

					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;

				case ST_MEN_MODE_CONFIG:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 0, "SELECCIONAR MODO:");
						displayUpdateRow(1, 0, "MODO CONFIG");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						// PUT SYSTEM TASK, EV_SYS_MODE CONFIG_ON
						put_event_task_system(p_task_menu_cfg->ev_sys_config_on);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_TIME;

					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->mode_time = false;
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_SENSOR;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;

				case ST_MEN_MODE_SENSOR:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(1, 0, "MODO SENSOR");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						// funcion que modifique la varible de system mod _sensr a true
						put_mode_task_system(&p_task_menu_dta->mode_time);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->mode_time = true;
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_TIME;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;

				case ST_MEN_MODE_TIME:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(1, 0, "MODO TIEMPO");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						put_mode_task_system(&p_task_menu_dta->mode_time);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_MANUAL;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;

				case ST_MEN_RIEGO_OFF:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(1, 0, "DESACTIVAR");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_riego_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_RIEGO_ON;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_MANUAL;
					}

					break;

				case ST_MEN_RIEGO_ON:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 0, "RIEGO");
						displayUpdateRow(1, 0, "ACTIVAR");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_riego_on);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;

					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_RIEGO_OFF;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_MANUAL;
					}

					break;

				case ST_MEN_CONFIG_TIME:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(0, 0, "VARIABLE A CONFIGURAR");
						displayUpdateRow(1, 0, "TIEMPO");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->tick_idle = 0; // tengoq eu ahce que se quede con el valor la variable configurada
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CHANGE_TIME;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_TEMP;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;

				case ST_MEN_CONFIG_TEMP:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(1, 0, "TEMPERATURA");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->threshold_temperature = 0; // tengoq eu ahce que se quede con el valor la variable configurada
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CHANGE_TEMP;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_HUME;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;

				case ST_MEN_CONFIG_HUME:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						displayUpdateRow(1, 0, "HUMEDAD");
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->threshold_humidity = 0; // tengoq eu ahce que se quede con el valor la variable configurada
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CHANGE_HUME;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_TIME;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;

				case ST_MEN_CHANGE_TIME:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						snprintf(str_buffer, sizeof(str_buffer), "TIEMPO: %-8lu", (p_task_menu_dta->tick_idle));
						displayUpdateRow(1, 0, str_buffer);
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						update_dta_task_system(&p_task_menu_dta->tick_idle, &p_task_menu_dta->threshold_temperature, &p_task_menu_dta->threshold_humidity);
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->tick_idle = 50 + (p_task_menu_dta->tick_idle + 1) % 10;
						p_task_menu_cfg->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_TIME;
					}

					break;

				case ST_MEN_CHANGE_TEMP:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						snprintf(str_buffer, sizeof(str_buffer), "TEMPE: %-8lu", (p_task_menu_dta->threshold_temperature));
						displayUpdateRow(1, 0, str_buffer);
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						update_dta_task_system(&p_task_menu_dta->tick_idle, &p_task_menu_dta->threshold_temperature, &p_task_menu_dta->threshold_humidity);
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->threshold_temperature= 10 + ((p_task_menu_dta->threshold_temperature + 1) % 10);
						p_task_menu_cfg->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_TEMP;
					}

					break;

				case ST_MEN_CHANGE_HUME:
					if (true == p_task_menu_cfg->refresh_screen)
					{
						p_task_menu_cfg->refresh_screen = false;
						snprintf(str_buffer, sizeof(str_buffer), "HUMEDAD: %-8lu", (p_task_menu_dta->threshold_humidity));
						displayUpdateRow(1, 0, str_buffer);
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						update_dta_task_system(&p_task_menu_dta->tick_idle, &p_task_menu_dta->threshold_temperature, &p_task_menu_dta->threshold_humidity);
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->threshold_humidity = 30 + ((p_task_menu_dta->threshold_humidity + 1) % 10);
						p_task_menu_cfg->flag = false;
						p_task_menu_cfg->refresh_screen = true;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIG_HUME;
					}

					break;

				default:

					p_task_menu_cfg->tick  = DEL_MEN_XX_MIN;
					p_task_menu_dta->state = ST_MEN_MAIN;
					p_task_menu_dta->event = EV_MEN_ENT_IDLE;
					p_task_menu_cfg->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/
