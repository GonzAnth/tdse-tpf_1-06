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
 * @file   : task_menu_attribute.h
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

#ifndef TASK_INC_TASK_MENU_ATTRIBUTE_H_
#define TASK_INC_TASK_MENU_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/
/* Menu Statechart - State Transition Table */
/* 	------------------------+-----------------------+-----------------------+-----------------------+------------------------
 * 	| Current               | Event                 |                       | Next                  |                       |
 * 	| State                 | (Parameters)          | [Guard]               | State                 | Actions               |
 * 	|=======================+=======================+=======================+=======================+=======================|
 * 	| ST_MEN_XX_IDLE        | EV_MEN_MEN_ACTIVE     |                       | ST_MEN_XX_ACTIVE      |                       |
 * 	|                       |                       |                       |                       |                       |
 * 	|-----------------------+-----------------------+-----------------------+-----------------------+-----------------------|
 * 	| ST_MEN_XX_ACTIVE      | EV_MEN_MEN_IDLE       |                       | ST_MEN_XX_IDLE        |                       |
 * 	|                       |                       |                       |                       |                       |
 * 	------------------------+-----------------------+-----------------------+-----------------------+------------------------
 * 	ESTO SE DEBE CAMBIAR
 */

/* Events to excite Task Menu */
typedef enum task_menu_ev {EV_MEN_ENT_IDLE,
						   EV_MEN_ENT_ACTIVE,
						   EV_MEN_NEX_IDLE,
						   EV_MEN_NEX_ACTIVE,
						   EV_MEN_ESC_IDLE,
						   EV_MEN_ESC_ACTIVE,
						   EV_MEN_ADC_REQ_OK} task_menu_ev_t;

/* State of Task Menu */
typedef enum task_menu_st {ST_MEN_MAIN,
						   ST_MEN_SELECT_MODE,
						   ST_MEN_MODE_MANUAL,
						   ST_MEN_MODE_CONFIG,
						   ST_MEN_MODE_SENSOR,
						   ST_MEN_MODE_TIME,
						   ST_MEN_RIEGO_ON,
						   ST_MEN_RIEGO_OFF,
						   ST_MEN_CONFIG_TIME,
						   ST_MEN_CONFIG_TEMP,
						   ST_MEN_CONFIG_HUME,
						   ST_MEN_CHANGE_TIME,
						   ST_MEN_CHANGE_TEMP,
						   ST_MEN_CHANGE_HUME,
						   ST_MEN_SALUD_WAIT,
						   ST_MEN_SALUD_SHOW} task_menu_st_t;

typedef struct
{
	uint32_t			tick;
	bool				flag;
	uint32_t			tick_idle_max;
	uint32_t			tick_riego_max;

	task_menu_ev_t  	ev_sys_config_on; //eventos que genera el menua para el systema
	task_menu_ev_t  	ev_sys_config_off;
	task_menu_ev_t  	ev_sys_riego_on;
	task_menu_ev_t  	ev_sys_riego_off;
	task_menu_ev_t  	ev_sys_adc_req;
	task_menu_ev_t  	ev_sys_mod_time;
	task_menu_ev_t  	ev_sys_mod_sensor;
} task_menu_cfg_t;

typedef struct
{
	uint32_t		tick_idle;
	uint32_t		tick_riego;

	task_menu_st_t	state;
	task_menu_st_t  last_state;
	task_menu_ev_t	event;

	uint32_t		threshold_temperature;
	uint32_t		threshold_humidity;

	//Impresion display
	bool 			refresh_screen; //Se debe actualizar la pantalla
	bool			printing;		//Impresion por etapas
	uint16_t 		etapa_print;	//Etapa de impresion
	bool 			refresh_cursor;  //Se debe actualizar el cursor
	uint16_t		cursor_pos;		//posición del cursor
} task_menu_dta_t;

/********************** external data declaration ****************************/
extern task_menu_dta_t task_menu_dta;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_MENU_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
