/**
 * @file    task_menu_attribute.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Definiciones de estructuras de datos, estados y configuración del Menu.
 * @date    Jan 28, 2026
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
	bool 				refresh_screen;
	uint32_t			tick_idle_max;

	task_menu_ev_t  	ev_sys_config_on; //eventos que genera el menu al systema
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
	task_menu_st_t	state;
	task_menu_st_t  last_state;
	task_menu_ev_t	event;
	bool			mode_time;
	uint32_t		threshold_temperature;
	uint32_t		threshold_humidity;
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
