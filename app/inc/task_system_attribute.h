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

#ifndef TASK_INC_TASK_SYSTEM_ATTRIBUTE_H_
#define TASK_INC_TASK_SYSTEM_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdint.h>
#include <stdbool.h>

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
typedef enum task_system_ev {EV_SYS_RIEGO_ACT_ON,
						   EV_SYS_RIEGO_ACT_OFF,
						   EV_SYS_RIEGO_NACT_ON,
						   EV_SYS_RIEGO_NACT_OFF,
						   EV_SYS_READY_ON,
						   EV_SYS_READY_OFF,
						   EV_SYS_CHECK_OK,
						   EV_SYS_CHECK_OK_OFF,
						   EV_SYS_CHECK_NOT_OK,
						   EV_SYS_CHECK_NOT_OK_OFF,
						   EV_SYS_FALLA_ON,
						   EV_SYS_FALLA_OFF,
						   EV_SYS_CONFIG_ON,
						   EV_SYS_CONFIG_OFF,
						   EV_SYS_NCONFIG_ON,
						   EV_SYS_NCONFIG_OFF} task_system_ev_t;

/* State of Task Menu */
typedef enum task_system_st {ST_SYS_IDLE,
						   ST_SYS_RIEGO,
						   ST_SYS_MEASURE,
						   ST_SYS_CONFIG,
						   ST_SYS_WAITING,
						   ST_SYS_FALLA} task_system_st_t;

typedef struct
{
	uint32_t			tick;
	bool				flag;
	uint32_t			tick_idle_max;
	uint32_t			tick_riego_max;
	uint32_t			tick_falla_max;
	task_system_ev_t 	signal_up; //eventos que genera el sensor para el systema
	task_system_ev_t	signal_down; //eventos que genera el sensor para el systema
} task_system_cfg_t;


typedef struct
{
	uint32_t			tick_idle;
	uint32_t			tick_riego;
	uint32_t			tick_falla;
	task_system_st_t	state;
	task_system_ev_t	event;
	bool 				mod_time;
	bool				mod_sensor;
	bool				mod_config;
	uint32_t			temperature;
	uint32_t			humidity;
} task_system_dta_t;

/********************** external data declaration ****************************/
extern task_system_dta_t task_system_dta;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_MENU_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
