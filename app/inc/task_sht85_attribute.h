/**
 * @file    task_sht85_attribute.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Definiciones de estructuras de datos, estados y configuración del SHT85.
 * @date    Dic 15, 2025
 */

#ifndef TASK_INC_TASK_SHT85_ATTRIBUTE_H_
#define TASK_INC_TASK_SHT85_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdint.h>
#include <stdbool.h>

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/* Events to excite Task Menu */
typedef enum task_sht85_ev {EV_SEN_MEASURE_ON,
						   EV_SEN_MEASURE_OFF,
						   EV_SEN_MEASURE_OK,
						   EV_SEN_MEASURE_OK_OFF,
						   EV_SEN_MEASURE_NOT_OK,
						   EV_SEN_MEASURE_NOT_OK_OFF,
						   EV_SEN_MEASURE_READ,
						   EV_SEN_MEASURE_READ_OFF,
						   EV_SEN_FALLA_OK,
						   EV_SEN_FALLA_OK_OFF} task_sht85_ev_t;

/* State of Task Menu */
typedef enum task_sht85_st {ST_SEN_IDLE,
						   ST_SEN_MEASURE,
						   ST_SEN_WAITING,
						   ST_SEN_READY,
						   ST_SEN_CHECK,
						   ST_SEN_FALLA} task_sht85_st_t;

typedef struct
{
	uint32_t			tick;
	bool				flag;
	uint32_t			tick_means_max;
	task_sht85_ev_t  	ev_sys_falla; //eventos que genera el sensor para el systema
	task_sht85_ev_t		ev_sys_ready_on; //eventos que genera el sensor para el systema
	task_sht85_ev_t		ev_sys_check_ok; //eventos que genera el sensor para el systema
	task_sht85_ev_t		ev_sys_check_not_ok; //eventos que genera el sensor para el systema
} task_sht85_cfg_t;


typedef struct
{
	uint32_t			tick_means;
	task_sht85_st_t		state;
	task_sht85_ev_t		event;
	bool 				ready;
	bool 				measure_check;
	uint32_t			time;
	float				temperature;
	float				humidity;
} task_sht85_dta_t;

/********************** external data declaration ****************************/
extern task_sht85_dta_t task_sht85_dta;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_MENU_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
