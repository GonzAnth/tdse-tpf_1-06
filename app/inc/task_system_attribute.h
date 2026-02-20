/**
 * @file    task_system_attribute.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Definiciones de estructuras de datos, estados y configuración del Systema.
 * @date    Dic 15, 2025
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

/* Events to excite Task Menu */
typedef enum task_system_ev {EV_SYS_IDLE,
						     EV_SYS_RIEGO_ON,
						     EV_SYS_RIEGO_OFF,
						     EV_SYS_READY,
						     EV_SYS_CHECK_OK,
						     EV_SYS_CHECK_NOT_OK,
						     EV_SYS_FALLA,
						     EV_SYS_CONFIG_ON,
							 EV_SYS_CONFIG_NEW,
						     EV_SYS_CONFIG_OFF,
						     EV_SYS_ADC_REQ,
						     EV_SYS_ADC_OK,
						     EV_SYS_ADC_NOT_OK,
						     EV_SYS_MOD_MANUAL,
						     EV_SYS_MOD_TIME,
						     EV_SYS_MOD_SENSOR} task_system_ev_t;

/* State of Task Menu */
typedef enum task_system_st {ST_SYS_IDLE,
						   ST_SYS_RIEGO,
						   ST_SYS_MEASURE,
						   ST_SYS_CONFIG,
						   ST_SYS_WAITING,
						   ST_SYS_FALLA,
						   ST_SYS_ADC_MEASURE} task_system_st_t;

/* Mode  of Task Menu */
typedef enum task_system_mode{SYS_MOD_MANUAL,
							SYS_MOD_TIME,
						    SYS_MOD_SENSOR} task_system_mode_t;

typedef struct
{
	uint32_t			tick;
	bool				flag;
	uint32_t			tick_idle_max;
	uint32_t			tick_riego_max;
	uint32_t			tick_falla_max;

	task_system_mode_t  system_mode;
	uint32_t			threshold_temperature;
	uint32_t			threshold_humidity;
	uint32_t			threshold_adc_temperature;
	uint32_t			threshold_adc_batery;
} task_system_cfg_t;


typedef struct
{
	uint32_t			tick_idle;
	uint32_t			tick_riego;
	uint32_t			tick_falla;
	task_system_st_t	state;
	task_system_st_t	last_state;
	task_system_ev_t	event;

	float				temperature;
	float				humidity;

	bool 				adc_req_pending;
	float				adc_temperature;
	float				adc_batery;
} task_system_dta_t;

/********************** external data declaration ****************************/
extern task_system_dta_t task_system_dta;
extern task_system_cfg_t task_system_cfg;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_SYSTEM_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
