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
#define DEL_MEN_XX_MED						5ul
#define DEL_MEN_XX_MAX						10ul

#define DEL_MEN_USER_FEEDBACK_MAX			200ul
#define DEL_MEN_USER_FEEDBACK_MIN			0ul

#define DEL_MEN_USER_FEEDBACK_FALLA_MAX		DEL_MEN_USER_FEEDBACK_MAX * 8
#define DEL_MEN_USER_FEEDBACK_FALLA_MIN		0ul

/********************** internal data declaration ****************************/
task_menu_cfg_t task_menu_cfg = {
	DEL_MEN_XX_MIN, false,
	DEL_MEN_USER_FEEDBACK_MAX,
	EV_SYS_CONFIG_ON, EV_SYS_NCONFIG_ON, EV_SYS_RIEGO_ACT_ON, EV_SYS_RIEGO_NACT_ON, EV_SYS_ADC_REQ,
	EV_SYS_MOD_MANUAL, EV_SYS_MOD_TIME, EV_SYS_MOD_SENSOR
};

task_menu_dta_t task_menu_dta = {
	DEL_MEN_USER_FEEDBACK_MAX,
	ST_MEN_MAIN, ST_MEN_MAIN, EV_MEN_ENT_IDLE,
	SYS_MOD_MANUAL,	0, 0, 0, 0.0, 0.0, false,
	true, false, 0, false, 0
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
					&p_task_menu_dta->sys_tick_idle,
					&p_task_menu_dta->sys_th_temperature,
					&p_task_menu_dta->sys_th_humidity);


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
					get_system_config(&p_task_menu_dta->sys_mode, NULL, NULL, NULL);
					p_task_menu_dta->sys_riego_state = get_system_riego_state();
				}
			    else if (p_task_menu_dta->state == ST_MEN_MODE_CONFIG)
				{
			    	get_system_config(&p_task_menu_dta->sys_mode,
			    					&p_task_menu_dta->sys_tick_idle,
			    					&p_task_menu_dta->sys_th_temperature,
			    					&p_task_menu_dta->sys_th_humidity);
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
						p_task_menu_dta->cursor_pos = (p_task_menu_dta->cursor_pos + 1) % 3;
						p_task_menu_dta->refresh_cursor = true;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						put_event_task_system(p_task_menu_cfg->ev_sys_config_on);
						switch (p_task_menu_dta->cursor_pos) {
							case 0: p_task_menu_dta->state = ST_MEN_CHANGE_TIME; break;
							case 1: p_task_menu_dta->state = ST_MEN_CHANGE_TEMP; break;
							case 2: p_task_menu_dta->state = ST_MEN_CHANGE_HUME; break;
						}
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_SELECT_MODE;
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
						p_task_menu_dta->state = ST_MEN_MODE_MANUAL;
					}

					break;


				case ST_MEN_CHANGE_TIME:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->sys_tick_idle = (p_task_menu_dta->sys_tick_idle + 10) % 60;
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						set_system_config(p_task_menu_dta->sys_tick_idle, p_task_menu_dta->sys_th_temperature, p_task_menu_dta->sys_th_humidity);
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						//p_task_menu_dta->tick_st_feedback_user = p_task_menu_cfg->tick_st_feedback_user_max;
						//LO MANDO A STATE DE SAVE OK O EL SYSTEMA ME CONFIRMA EL CAMBIO
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_CHANGE_TEMP:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->sys_th_temperature = (p_task_menu_dta->sys_th_temperature + 1) % 50;
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						set_system_config(p_task_menu_dta->sys_tick_idle, p_task_menu_dta->sys_th_temperature, p_task_menu_dta->sys_th_humidity);
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						//p_task_menu_dta->tick_st_feedback_user = p_task_menu_cfg->tick_st_feedback_user_max;
						//LO MANDO A STATE DE SAVE OK O EL SYSTEMA ME CONFIRMA EL CAMBIO
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
					}

					break;


				case ST_MEN_CHANGE_HUME:
					if ((true == p_task_menu_cfg->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_dta->sys_th_humidity = (p_task_menu_dta->sys_th_humidity + 5) % 100;
						p_task_menu_dta->refresh_screen = true;
						p_task_menu_cfg->flag = false;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
					{
						set_system_config(p_task_menu_dta->sys_tick_idle, p_task_menu_dta->sys_th_temperature, p_task_menu_dta->sys_th_humidity);
						put_event_task_system(p_task_menu_cfg->ev_sys_config_off);
						//p_task_menu_dta->tick_st_feedback_user = p_task_menu_cfg->tick_st_feedback_user_max;
						//LO MANDO A STATE DE SAVE OK O EL SYSTEMA ME CONFIRMA EL CAMBIO
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MAIN;
					}
					else if ((true == p_task_menu_cfg->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
					{
						p_task_menu_cfg->flag = false;
						p_task_menu_dta->state = ST_MEN_MODE_CONFIG;
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
	char str_buffer[ANCHO_LCD + 1]; //se suma caracter \0

	if (true == dta->refresh_screen)
	{
		dta->refresh_screen = false;
		dta->printing = true;
		dta->etapa_print = 0;
		dta->refresh_cursor = true;
	}

	switch (dta->state){
		case ST_MEN_MAIN:
			if (true == dta->printing)
			{
				const char *text_mode = "ERROR";
				switch (dta->sys_mode) {
				    case SYS_MOD_MANUAL: text_mode = "MODO ACTUAL: MANUAL";	 break;
				    case SYS_MOD_SENSOR: text_mode = "MODO: SENSOR & TIMER"; break;
				    case SYS_MOD_TIME: 	 text_mode = "MODO ACTUAL: TIMER"; 	 break;
				    default: break;
				}
				const char *text_riego = (dta->sys_riego_state) ? "BOMBA ENCENDIDA" : "BOMBA APAGADA";

				switch (dta->etapa_print)
				{
					case 0: displayRowSplit(0, 1, "Trabajo Final TDSE", PART_LEFT); break;
					case 1: displayRowSplit(0, 1, "Trabajo Final TDSE", PART_RIGHT); break;
					//case 2: displayRowSplit(1, 6, "Grupo: 6", PART_LEFT); break;
					//case 3: displayRowSplit(1, 6, "Grupo: 6", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 0, text_mode, PART_LEFT); break;
					case 5: displayRowSplit(2, 0, text_mode, PART_RIGHT); break;
					case 6: displayRowSplit(3, 0, text_riego, PART_LEFT); break;
					case 7: displayRowSplit(3, 0, text_riego, PART_RIGHT); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_SALUD_WAIT:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayRowSplit(0, 1, "ESTADO DEL EQUIPO:", PART_LEFT); break;
					case 1: displayRowSplit(0, 1, "ESTADO DEL EQUIPO:", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 0, "Midiendo ...", PART_LEFT); break;
					case 5: displayRowSplit(2, 0, "Midiendo ...", PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_SALUD_SHOW:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0:
					case 1:
						int bat_e = (int)dta->sys_salud_bat_v;
						int bat_d = (int)(fabs(dta->sys_salud_bat_v - bat_e) * 100);
						snprintf(str_buffer, sizeof(str_buffer), "Bateria: %d.%02d V", bat_e, bat_d);
						displayRowSplit(2, 0, str_buffer, (dta->etapa_print == 0) ? PART_LEFT : PART_RIGHT);
						break;

					case 2:
					case 3:
						int temp_e = (int)dta->sys_salud_temp_int_c;
						int temp_d = (int)(fabs(dta->sys_salud_temp_int_c - temp_e) * 100);
						snprintf(str_buffer, sizeof(str_buffer), "T. interna: %d.%02d C", temp_e, temp_d);
						displayRowSplit(3, 0, str_buffer, (dta->etapa_print == 2) ? PART_LEFT : PART_RIGHT);
						break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_SELECT_MODE:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayRowSplit(0, 1, "MODO MANUAL", PART_LEFT); break;
					case 1: displayRowSplit(0, 1, "MODO MANUAL", PART_RIGHT); break;
					case 2: displayRowSplit(1, 1, "MODO SENSOR & TIMER", PART_LEFT); break;
					case 3: displayRowSplit(1, 1, "MODO SENSOR & TIMER", PART_RIGHT); break;
					case 4: displayRowSplit(2, 1, "MODO TIMER", PART_LEFT); break;
					case 5: displayRowSplit(2, 1, "MODO TIMER", PART_RIGHT); break;
					case 6: displayRowSplit(3, 1, "CONFIGURAR EQUIPO", PART_LEFT); break;
					case 7: displayRowSplit(3, 1, "CONFIGURAR EQUIPO", PART_RIGHT); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			if ((false == dta->printing) && (true == dta->refresh_cursor))
			{
				dta->refresh_cursor = false;
				for (int i=0; i<4; i++) {
					displayCharPositionWrite(0, i);
					displayStringWrite(" ");
				}
				displayCharPositionWrite(0, dta->cursor_pos);
				displayStringWrite(">");
			}
			break;


		case ST_MEN_MODE_MANUAL:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayRowSplit(0, 0, "MODO MANUAL ACTIVADO", PART_LEFT); break;
					case 1: displayRowSplit(0, 0, "MODO MANUAL ACTIVADO", PART_RIGHT); break;
					case 2: displayRowSplit(1, 4, "SELECCIONAR:", PART_LEFT); break;
					case 3: displayRowSplit(1, 4, "SELECCIONAR:", PART_RIGHT); break;
					case 4: displayRowSplit(2, 1, "DESACTIVAR RIEGO", PART_LEFT); break;
					case 5: displayRowSplit(2, 1, "DESACTIVAR RIEGO", PART_RIGHT); break;
					case 6: displayRowSplit(3, 1, "ACTIVAR RIEGO", PART_LEFT); break;
					case 7: displayRowSplit(3, 1, "ACTIVAR RIEGO", PART_RIGHT); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			if ((false == dta->printing) && (true == dta->refresh_cursor))
			{
				dta->refresh_cursor = false;
				displayCharPositionWrite(0, 2); displayStringWrite(" ");
				displayCharPositionWrite(0, 3); displayStringWrite(" ");
				displayCharPositionWrite(0, dta->cursor_pos + 2);
				displayStringWrite(">");
			}
			break;


		case ST_MEN_RIEGO_OFF:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayRowSplit(0, 4, "MODO: MANUAL", PART_LEFT); break;
					case 1: displayRowSplit(0, 4, "MODO: MANUAL", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 3, "BOMBA APAGADA!", PART_LEFT); break;
					case 5: displayRowSplit(2, 3, "BOMBA APAGADA!", PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_RIEGO_ON:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayRowSplit(0, 4, "MODO: MANUAL", PART_LEFT); break;
					case 1: displayRowSplit(0, 4, "MODO: MANUAL", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 2, "BOMBA ENCENDIDA!", PART_LEFT); break;
					case 5: displayRowSplit(2, 2, "BOMBA ENCENDIDA!", PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_MODE_CONFIG:
			if (true == dta->printing) {
				switch (dta->etapa_print) {
					case 0: displayRowSplit(0, 1, "MODO CONFIGURACION", PART_LEFT); break;
					case 1: displayRowSplit(0, 1, "MODO CONFIGURACION", PART_RIGHT); break;
					case 2: displayRowSplit(1, 1, "CONFIG TIEMPO SLEEP", PART_LEFT);  break;
					case 3: displayRowSplit(1, 1, "CONFIG TIEMPO SLEEP", PART_RIGHT); break;
					case 4: displayRowSplit(2, 1, "CONFIG UMBRAL TEMP", PART_LEFT);  break;
					case 5: displayRowSplit(2, 1, "CONFIG UMBRAL TEMP", PART_RIGHT); break;
					case 6: displayRowSplit(3, 1, "CONFIG UMBRAL HUM", PART_LEFT);  break;
					case 7: displayRowSplit(3, 1, "CONFIG UMBRAL HUM", PART_RIGHT); break;
					default: dta->printing = false; break;
				}
				dta->etapa_print++;
			}
			if ((false == dta->printing) && (true == dta->refresh_cursor))
			{
				dta->refresh_cursor = false;
				displayCharPositionWrite(0, 1); displayStringWrite(" ");
				displayCharPositionWrite(0, 2); displayStringWrite(" ");
				displayCharPositionWrite(0, 3); displayStringWrite(" ");
				displayCharPositionWrite(0, dta->cursor_pos + 1);
				displayStringWrite(">");
			}
			break;


		case ST_MEN_CHANGE_TIME:
			if (true == dta->printing) {
				snprintf(str_buffer, sizeof(str_buffer), "NUEVO TIEMPO %-3lu min", dta->sys_tick_idle);
				switch (dta->etapa_print) {
					case 0: displayRowSplit(0, 0, "CONFIG. TIEMPO SLEEP", PART_LEFT); break;
					case 1: displayRowSplit(0, 0, "CONFIG. TIEMPO SLEEP", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 0, str_buffer, PART_LEFT);  break;
					case 5: displayRowSplit(2, 0, str_buffer, PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default: dta->printing = false; break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_CHANGE_TEMP:
			if (true == dta->printing) {
				snprintf(str_buffer, sizeof(str_buffer), "NUEVA TEMP %-3lu C", dta->sys_th_temperature);
				switch (dta->etapa_print) {
					case 0: displayRowSplit(0, 1, "CONFIG UMBRAL TEMP", PART_LEFT); break;
					case 1: displayRowSplit(0, 1, "CONFIG UMBRAL TEMP", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 2, str_buffer, PART_LEFT);  break;
					case 5: displayRowSplit(2, 2, str_buffer, PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default: dta->printing = false; break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_CHANGE_HUME:
			if (true == dta->printing) {
				snprintf(str_buffer, sizeof(str_buffer), "NUEVA HUM: %-3lu %%", dta->sys_th_humidity);
				switch (dta->etapa_print) {
					case 0: displayRowSplit(0, 1, "CONFIG. UMBRAL HUM", PART_LEFT); break;
					case 1: displayRowSplit(0, 1, "CONFIG. UMBRAL HUM", PART_RIGHT); break;
					case 2: displayClearPart(1, 0, 10); break;
					case 3: displayClearPart(1, 10, 10); break;
					case 4: displayRowSplit(2, 2, str_buffer, PART_LEFT);  break;
					case 5: displayRowSplit(2, 2, str_buffer, PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default: dta->printing = false; break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_MODE_SENSOR:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayClearPart(0, 0, 10); break;
					case 1: displayClearPart(0, 10, 10); break;
					case 2: displayRowSplit(1, 0, "MODO SENSOR & TIMER", PART_LEFT); break;
					case 3: displayRowSplit(1, 0, "MODO SENSOR & TIMER", PART_RIGHT); break;
					case 4: displayRowSplit(2, 6, "ACTIVADO", PART_LEFT); break;
					case 5: displayRowSplit(2, 6, "ACTIVADO", PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_MODE_TIME:
			if (true == dta->printing)
			{
				switch (dta->etapa_print)
				{
					case 0: displayClearPart(0, 0, 10); break;
					case 1: displayClearPart(0, 10, 10); break;
					case 2: displayRowSplit(1, 5, "MODO TIMER", PART_LEFT); break;
					case 3: displayRowSplit(1, 5, "MODO TIMER", PART_RIGHT); break;
					case 4: displayRowSplit(2, 6, "ACTIVADO", PART_LEFT); break;
					case 5: displayRowSplit(2, 6, "ACTIVADO", PART_RIGHT); break;
					case 6: displayClearPart(3, 0, 10); break;
					case 7: displayClearPart(3, 10, 10); break;
					default:
						dta->printing = false;
						break;
				}
				dta->etapa_print++;
			}
			break;


		case ST_MEN_FALLA_SHOW:
			if (true == dta->printing) {
				switch (dta->etapa_print) {
					case 0: displayRowSplit(0, 4, "FATAL ERROR!", PART_LEFT); break;
					case 1: displayRowSplit(0, 4, "FATAL ERROR!", PART_RIGHT); break;
					case 2: displayRowSplit(1, 2, "REINTENTANDO ...", PART_LEFT); break;
					case 3: displayRowSplit(1, 2, "REINTENTANDO ...", PART_RIGHT); break;
					case 4: displayClearPart(2, 0, 10); break;
					case 5: displayClearPart(2, 10, 10); break;
					case 6: displayRowSplit(3, 2, "ESC/ENT: ACEPTAR", PART_LEFT); break;
					case 7: displayRowSplit(3, 2, "ESC/ENT: ACEPTAR", PART_RIGHT); break;
					default: dta->printing = false; break;
				}
				dta->etapa_print++;
			}
			break;

		default:
			dta->printing = false;
			break;

	}
}

/********************** end of file ******************************************/
