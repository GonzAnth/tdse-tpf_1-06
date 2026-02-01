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
#include <math.h>

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI					0ul
#define G_TASK_MEN_TICK_CNT_INI				0ul

#define DEL_MEN_XX_MIN						0ul
#define DEL_MEN_XX_MED						2ul
#define DEL_MEN_XX_MAX						4ul

#define DEL_MEN_USER_FEEDBACK_MAX			200ul
#define DEL_MEN_USER_FEEDBACK_MIN			0ul

#define DEL_MEN_USER_FEEDBACK_FALLA_MAX		DEL_MEN_USER_FEEDBACK_MAX * 8
#define DEL_MEN_USER_FEEDBACK_FALLA_MIN		0ul

#define IDLE_TIME_INCREMENT					15
#define IDLE_TIME_TOP						360
#define RIEGO_TIME_INCREMENT				5
#define RIEGO_TIME_TOP						60
#define TH_TEMP_INCREMENT					5
#define TH_TEMP_TOP							100
#define TH_HUM_INCREMENT					5
#define TH_HUM_TOP							100

#define NO_CURSOR							255

/********************** internal data declaration ****************************/
task_menu_cfg_t task_menu_cfg = {
	DEL_MEN_XX_MIN, false,
	DEL_MEN_USER_FEEDBACK_MAX,
	EV_SYS_CONFIG_ON, EV_SYS_NCONFIG_ON, EV_SYS_RIEGO_ACT_ON, EV_SYS_RIEGO_NACT_ON, EV_SYS_ADC_REQ,
	EV_SYS_MOD_MANUAL, EV_SYS_MOD_TIME, EV_SYS_MOD_SENSOR
};

task_menu_dta_t task_menu_dta = {
	.tick_st_feedback_user 	= DEL_MEN_USER_FEEDBACK_MAX,
	.state 					= ST_MEN_MAIN,
	.last_state 			= ST_MEN_MAIN,
	.event 					= EV_MEN_ENT_IDLE,

	.edit_changes			= false,

	.refresh_screen 		= true,
	.printing				= false,
	.refresh_cursor			= false,
};


#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

static void menu_display_print(task_menu_dta_t *dta);

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

	/* Obtenemos situación actual del systema */
	get_system_config(&p_task_menu_dta->sys_mode,
					&p_task_menu_dta->edit_sys_tick_idle,
					&p_task_menu_dta->edit_sys_tick_riego,
					&p_task_menu_dta->edit_sys_th_temperature,
					&p_task_menu_dta->edit_sys_th_humidity);


	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	task_menu_cfg_t *p_task_menu_cfg;
	task_menu_dta_t *p_task_menu_dta;

	bool b_time_update_required = false;

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

			menu_display_print(p_task_menu_dta);


			/* Aquí colocamos código a ejecutar cuando cambiamos de estado */
			if (p_task_menu_dta->state != p_task_menu_dta->last_state)
			{
			    p_task_menu_dta->refresh_screen = true;
			    p_task_menu_dta->cursor_pos = 0;
			    p_task_menu_dta->last_state = p_task_menu_dta->state;

			    if (p_task_menu_dta->state == ST_MEN_MAIN) {
					get_system_config(&p_task_menu_dta->sys_mode, NULL, NULL, NULL, NULL);
					p_task_menu_dta->sys_riego_state = get_system_riego_state();
				}
			    else if (p_task_menu_dta->state == ST_MEN_MODE_CONFIG)
				{
			    	if (false == p_task_menu_dta->edit_changes) {
						get_system_config(NULL,
										&p_task_menu_dta->edit_sys_tick_idle,
										&p_task_menu_dta->edit_sys_tick_riego,
										&p_task_menu_dta->edit_sys_th_temperature,
										&p_task_menu_dta->edit_sys_th_humidity);
					}
				}
			    else if (p_task_menu_dta->state == ST_MEN_SALUD_SHOW)
				{
			    	get_system_salud_dta(&p_task_menu_dta->sys_salud_bat_v,
										&p_task_menu_dta->sys_salud_temp_int_c);
				}
			}


			/* Implementacion maquina de estados */
			if (true == any_event_task_menu())
			{
				p_task_menu_cfg->flag = true;
				p_task_menu_dta->event = get_event_task_menu();

				/* EVENTO DE MÁXIMA PRIORIDAD */
				if (EV_MEN_SYS_FALLA == p_task_menu_dta->event)
				{
					p_task_menu_dta->tick_st_feedback_user = DEL_MEN_USER_FEEDBACK_FALLA_MAX;
					p_task_menu_dta->state = ST_MEN_FALLA_SHOW;
					p_task_menu_cfg->flag = false;
				}
			}

			switch (p_task_menu_dta->state)
			{
				case ST_MEN_MAIN:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_SELECT_MODE;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_adc_req);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_SALUD_WAIT;
					}

					/* impresión estado bomba */
					bool actual_sys_riego_state = get_system_riego_state();
					if (actual_sys_riego_state != p_task_menu_dta->sys_riego_state)
					{
						p_task_menu_dta->sys_riego_state = actual_sys_riego_state;
						p_task_menu_dta->refresh_screen = true;
					}

					break;


				case ST_MEN_SALUD_WAIT:
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
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;


				case ST_MEN_SELECT_MODE:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->cursor_pos = (p_task_menu_dta->cursor_pos + 1) % 4;
						p_task_menu_dta->refresh_cursor = true;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->tick_st_feedback_user = p_task_menu_cfg->tick_st_feedback_user_max;
						switch (p_task_menu_dta->cursor_pos) {
							case 0:
								put_event_task_system(p_task_menu_cfg->ev_sys_mod_manual);
								p_task_menu_dta->state = ST_MEN_MODE_MANUAL;
								break;
							case 1:
								put_event_task_system(p_task_menu_cfg->ev_sys_mod_sensor);
								p_task_menu_dta->state = ST_MEN_MODE_SENSOR;
								break;
							case 2:
								put_event_task_system(p_task_menu_cfg->ev_sys_mod_time);
								p_task_menu_dta->state = ST_MEN_MODE_TIME;
								break;
							case 3: p_task_menu_dta->state = ST_MEN_MODE_CONFIG; break;
						}
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

		            break;


				case ST_MEN_MODE_MANUAL:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->cursor_pos = (p_task_menu_dta->cursor_pos + 1) % 2;
						p_task_menu_dta->refresh_cursor = true;
					}

					if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->tick_st_feedback_user = p_task_menu_cfg->tick_st_feedback_user_max;
						switch (p_task_menu_dta->cursor_pos) {
							case 0:
								put_event_task_system(p_task_menu_cfg->ev_sys_riego_off);
								p_task_menu_dta->state = ST_MEN_RIEGO_OFF;
								break;
							case 1:
								put_event_task_system(p_task_menu_cfg->ev_sys_riego_on);
								p_task_menu_dta->state = ST_MEN_RIEGO_ON;
								break;
						}

					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_SELECT_MODE;
					}

					break;


				case ST_MEN_MODE_CONFIG:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->cursor_pos = (p_task_menu_dta->cursor_pos + 1) % 4;
						p_task_menu_dta->refresh_cursor = true;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						put_event_task_system(p_task_menu_cfg->ev_sys_config_on);
						switch (p_task_menu_dta->cursor_pos) {
							case 0:
								p_task_menu_dta->edit_val_backup = p_task_menu_dta->edit_sys_tick_idle;
								p_task_menu_dta->state =ST_MEN_CHANGE_IDLE_TIME;
								break;
							case 1:
								p_task_menu_dta->edit_val_backup = p_task_menu_dta->edit_sys_tick_riego;
								p_task_menu_dta->state = ST_MEN_CHANGE_RIEGO_TIME;
								break;
							case 2:
								p_task_menu_dta->edit_val_backup = p_task_menu_dta->edit_sys_th_temperature;
								p_task_menu_dta->state = ST_MEN_CHANGE_TEMP;
								break;
							case 3:
								p_task_menu_dta->edit_val_backup = p_task_menu_dta->edit_sys_th_humidity;
								p_task_menu_dta->state = ST_MEN_CHANGE_HUME;
								break;
						}
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						if (true == p_task_menu_dta->edit_changes){
							p_task_menu_dta->state = ST_MEN_CONFIRM_CONFIG;
						} else {
							put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
							p_task_menu_dta->state = ST_MEN_SELECT_MODE;
						}

					}

					break;


				case ST_MEN_MODE_SENSOR:
					p_task_menu_dta->tick_st_feedback_user--;
					if (((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)) || (DEL_MEN_USER_FEEDBACK_MIN == p_task_menu_dta->tick_st_feedback_user))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;


				case ST_MEN_MODE_TIME:
					p_task_menu_dta->tick_st_feedback_user--;
					if (((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)) || (DEL_MEN_USER_FEEDBACK_MIN == p_task_menu_dta->tick_st_feedback_user))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;


				case ST_MEN_RIEGO_OFF:
				case ST_MEN_RIEGO_ON:
					p_task_menu_dta->tick_st_feedback_user--;
					if (((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)) || (DEL_MEN_USER_FEEDBACK_MIN == p_task_menu_dta->tick_st_feedback_user))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;


				case ST_MEN_CHANGE_IDLE_TIME:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_tick_idle += IDLE_TIME_INCREMENT;
						if (p_task_menu_dta->edit_sys_tick_idle > IDLE_TIME_TOP)
						{
							p_task_menu_dta->edit_sys_tick_idle = IDLE_TIME_INCREMENT;
						}
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_changes = true;

						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIRM_CONFIG;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_tick_idle = p_task_menu_dta->edit_val_backup;
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_CHANGE_RIEGO_TIME:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_tick_riego += RIEGO_TIME_INCREMENT;
						if (p_task_menu_dta->edit_sys_tick_riego > RIEGO_TIME_TOP)
						{
							p_task_menu_dta->edit_sys_tick_riego = RIEGO_TIME_INCREMENT;
						}
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_changes = true;

						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIRM_CONFIG;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_tick_riego = p_task_menu_dta->edit_val_backup;
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_CHANGE_TEMP:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_th_temperature += TH_TEMP_INCREMENT;
						if (p_task_menu_dta->edit_sys_th_temperature > TH_TEMP_TOP)
						{
							p_task_menu_dta->edit_sys_th_temperature = TH_TEMP_INCREMENT;
						}
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_changes = true;

						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIRM_CONFIG;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_th_temperature = p_task_menu_dta->edit_val_backup;
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_CHANGE_HUME:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_th_humidity += TH_HUM_INCREMENT;
						if (p_task_menu_dta->edit_sys_th_humidity > TH_HUM_TOP)
						{
							p_task_menu_dta->edit_sys_th_humidity = TH_HUM_INCREMENT;
						}
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_changes = true;

						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_CONFIRM_CONFIG;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->edit_sys_th_humidity = p_task_menu_dta->edit_val_backup;
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_CONFIRM_CONFIG:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)) {
						p_task_menu_dta->cursor_pos = (p_task_menu_dta->cursor_pos + 1) % 3;
						p_task_menu_dta->refresh_cursor = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						switch (p_task_menu_dta->cursor_pos) {
							case 0:
								set_system_config(p_task_menu_dta->edit_sys_tick_idle,
												  p_task_menu_dta->edit_sys_tick_riego,
												  p_task_menu_dta->edit_sys_th_temperature,
												  p_task_menu_dta->edit_sys_th_humidity);
								put_event_task_system(p_task_menu_cfg->ev_sys_config_off);

								p_task_menu_dta->edit_changes = false;
								p_task_menu_dta->tick_st_feedback_user = DEL_MEN_USER_FEEDBACK_MAX;
								p_task_menu_dta->state = ST_MEN_SAVE_CONFIG;
								break;

							case 1: p_task_menu_dta->state = ST_MEN_MODE_CONFIG; break;
							case 2:
								p_task_menu_dta->edit_changes = false;
								p_task_menu_dta->state = ST_MEN_SELECT_MODE;
								break;
						}
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)) {
						 p_task_menu_cfg->flag = false;
						 p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_SAVE_CONFIG:
					p_task_menu_dta->tick_st_feedback_user--;
					if (p_task_menu_dta->tick_st_feedback_user == DEL_MEN_USER_FEEDBACK_MIN)
					{
						p_task_menu_dta->state = ST_MEN_MAIN;
					}

					break;


				case ST_MEN_FALLA_SHOW:
					p_task_menu_dta->tick_st_feedback_user--;
					if ((p_task_menu_dta->tick_st_feedback_user == DEL_MEN_USER_FEEDBACK_FALLA_MIN) ||
					   ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)) ||
					   ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)))
					{
						p_task_menu_dta->state = ST_MEN_MAIN;
						p_task_menu_cfg->flag = false;
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


static void menu_display_print(task_menu_dta_t *dta){
	if (true == dta->refresh_screen)
	{
		dta->refresh_screen = false;
		dta->printing = true;
		dta->etapa_print = 0;
		dta->refresh_cursor = true;

		//Lineas vacias
		dta->lines[0] = "";
		dta->lines[1] = "";
		dta->lines[2] = "";
		dta->lines[3] = "";

		switch (dta->state){
			case ST_MEN_MAIN:
				dta->lines[0] = " Trabajo Final TDSE ";

				if (dta->sys_mode == SYS_MOD_MANUAL) dta->lines[2] = "MODO: MANUAL";
				else if (dta->sys_mode == SYS_MOD_SENSOR) dta->lines[2] = "MODO: SENSOR";
				else dta->lines[2] = "MODO: TIMER";

				if (dta->sys_riego_state) dta->lines[3] = ">>  BOMBA: ON";
				else dta->lines[3] = ">>  BOMBA: OFF";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_SALUD_WAIT:
				dta->lines[0]		= " ESTADO DEL EQUIPO: ";
				if (true == dta->sys_riego_state)
				{
					dta->lines[2]	= "  SYSTEMA OCUPADO!  ";
					dta->lines[3]	= "REINTENTAR MAS TARDE";
				} else {
					dta->lines[2]	= "Midiendo ...        ";
				}

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_SALUD_SHOW:
				dta->lines[0] = " ESTADO DEL EQUIPO: ";

				int bat_e = (int)dta->sys_salud_bat_v;
				int bat_d = (int)(fabs(dta->sys_salud_bat_v - bat_e) * 100);
				snprintf(dta->aux_str_buf, ANCHO_LCD+1, "Bateria: %d.%02d V", bat_e, bat_d);
				dta->lines[2] = dta->aux_str_buf;


				int temp_e = (int)dta->sys_salud_temp_int_c;
				int temp_d = (int)(fabs(dta->sys_salud_temp_int_c - temp_e) * 100);
				snprintf(dta->aux_str_buf_2, 21, "T. interna: %d.%02d C", temp_e, temp_d);
				dta->lines[3] = dta->aux_str_buf_2;

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_SELECT_MODE:
				dta->lines[0] = " MODO MANUAL";
				dta->lines[1] = " MODO SENSOR & TIMER";
				dta->lines[2] = " MODO TIMER";
				dta->lines[3] = " CONFIGURAR EQUIPO";

				dta->cursor_offset = 0;
				break;


			case ST_MEN_MODE_MANUAL:
				dta->lines[0] = "MODO MANUAL ACTIVADO";
				dta->lines[1] = "    SELECCIONAR:    ";
				dta->lines[2] = " DESACTIVAR RIEGO";
				dta->lines[3] = " ACTIVAR RIEGO";

				dta->cursor_offset = 2;
				break;


			case ST_MEN_RIEGO_OFF:
				dta->lines[0] = "    MODO: MANUAL    ";
				dta->lines[2] = "   BOMBA APAGADA!   ";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_RIEGO_ON:
				dta->lines[0] = "    MODO: MANUAL    ";
				dta->lines[2] = "  BOMBA ENCENDIDA!  ";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_MODE_CONFIG:
				dta->lines[0] = " CONFIG TIEMPO SLEEP";
				dta->lines[1] = " CONFIG TIEMPO RIEGO";
				dta->lines[2] = " CONFIG UMBRAL TEMP";
				dta->lines[3] = " CONFIG UMBRAL HUM";

				dta->cursor_offset = 0;
				break;


			case ST_MEN_CHANGE_IDLE_TIME:
				dta->lines[0] = "CONFIG. TIEMPO SLEEP";
				snprintf(dta->aux_str_buf, ANCHO_LCD+1, "NUEVO TIEMPO:%lu min", dta->edit_sys_tick_idle);
				dta->lines[2] = dta->aux_str_buf;
				dta->lines[3] = "ENT: OK  ESC: CANCEL";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_CHANGE_RIEGO_TIME:
				dta->lines[0] = "CONFIG. TIEMPO RIEGO";
				snprintf(dta->aux_str_buf, ANCHO_LCD+1, "NUEVO TIEMPO:%lu min", dta->edit_sys_tick_riego);
				dta->lines[2] = dta->aux_str_buf;
				dta->lines[3] = "ENT: OK  ESC: CANCEL";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_CHANGE_TEMP:
				dta->lines[0] = " CONFIG UMBRAL TEMP ";
				snprintf(dta->aux_str_buf, ANCHO_LCD+1, "NUEVA TEMP: %lu C", dta->edit_sys_th_temperature);
				dta->lines[2] = dta->aux_str_buf;
				dta->lines[3] = "ENT: OK  ESC: CANCEL";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_CHANGE_HUME:
				dta->lines[0] = " CONFIG UMBRAL HUM  ";
				snprintf(dta->aux_str_buf, ANCHO_LCD+1, "NUEVA HUM: %lu %%", dta->edit_sys_th_humidity);
				dta->lines[2] = dta->aux_str_buf;
				dta->lines[3] = "ENT: OK  ESC: CANCEL";

				dta->cursor_offset = NO_CURSOR;
				break;



			case ST_MEN_CONFIRM_CONFIG:
				dta->lines[0] = " CONFIRMAR CAMBIOS? ";
				dta->lines[1] = "  GUARDAR Y SALIR";
				dta->lines[2] = "  SEGUIR EDITANDO";
				dta->lines[3] = "  DESCARTAR";

				dta->cursor_offset = 1;
				break;


			case ST_MEN_SAVE_CONFIG:
				dta->lines[1] = "   MODIFICACIONES   ";
				dta->lines[2] = "     APLICADAS!     ";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_MODE_SENSOR:
				dta->lines[1] = "MODO SENSOR & TIMER ";
				dta->lines[2] = "      ACTIVADO      ";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_MODE_TIME:
				dta->lines[1] = "     MODO TIMER     ";
				dta->lines[2] = "      ACTIVADO      ";

				dta->cursor_offset = NO_CURSOR;
				break;


			case ST_MEN_FALLA_SHOW:
				dta->lines[0] = "    FATAL ERROR!    ";
				dta->lines[1] = "  REINTENTANDO ...  ";
				dta->lines[3] = "  ENT/ESC: ACEPTAR  ";

				dta->cursor_offset = NO_CURSOR;
				break;


			default:
				dta->lines[0] = "ERR: PANTALLA VACIA ";
				dta->cursor_offset = NO_CURSOR;
				break;
		}

		return; //para ejecutar lo siguiente en otro tick
	}



	if (true == dta->printing)
	{
		if (dta->etapa_print < 16)
		{
			uint8_t fila  = dta->etapa_print / 4;
			uint8_t chunk = dta->etapa_print % 4;
			displayPrintPart(fila, chunk, dta->lines[fila]);

			dta->etapa_print++;
		}
		else
		{
			dta->printing = false;
		}
		return;
	}



	if ((false == dta->printing) && (true == dta->refresh_cursor))
	{
		dta->refresh_cursor = false;
		if (dta->cursor_offset != NO_CURSOR)
		{
			for(int i=dta->cursor_offset; i<4; i++) {
				displayCharPositionWrite(0, i);
				displayStringWrite(" ");
			}

			uint8_t fila_real = dta->cursor_pos + dta->cursor_offset;
			if (fila_real > 3) fila_real = 3;

			displayCharPositionWrite(0, fila_real);
			displayStringWrite(">");
		}
	}
}



/********************** end of file ******************************************/
