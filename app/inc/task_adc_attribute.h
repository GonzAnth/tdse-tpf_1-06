/*
 * task_adc.h
 *
 *  Created on: Jan 24, 2026
 *      Author: Gonzalo
 */

#ifndef INC_TASK_ADC_ATTRIBUTE_H_
#define INC_TASK_ADC_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdint.h>
#include <stdbool.h>
/********************** macros ***********************************************/

#define VREFINT_CAL_VOLTS   		1.20f
#define ADC_MAX_COUNT       		4095.0f
#define TEMP_V25            		1.43f
#define TEMP_AVG_SLOPE      		0.0043f


/********************** typedef **********************************************/

/* Events to excite Task adc */
typedef enum task_adc_ev {EV_MEN_ENT_IDLE,
						   EV_MEN_ENT_ACTIVE,
						   EV_MEN_NEX_IDLE,
						   EV_MEN_NEX_ACTIVE,
						   EV_MEN_ESC_IDLE,
						   EV_MEN_ESC_ACTIVE} task_adc_ev_t;

/* State of Task adc */
typedef enum task_adc_st {ST_ADC_IDLE,
							ST_ADC_TEMP_START,
							ST_ADC_TEMP_WAITING,
							ST_ADC_BAT_START,
							ST_ADC_BAT_WAITING,
							ST_ADC_FALLA} task_adc_st_t;

typedef struct
{
	uint32_t			tick;
	bool				flag;
	uint32_t			tick_idle_max;
	task_adc_ev_t  		ev_sys_config_on; //eventos que genera el adca para el systema
	task_adc_ev_t  		ev_sys_config_off;
	task_adc_ev_t  		ev_sys_riego_on;
	task_adc_ev_t  		ev_sys_riego_off;
} task_adc_cfg_t;

typedef struct
{
	uint32_t 			tick_adc;
	task_adc_st_t		state;
	task_adc_ev_t		event;

	bool 				flag_ready;
	volatile uint16_t 	last_raw_lecture;

	uint16_t			temp_raw;
	float				temp_cent;
	uint16_t			bat_raw;
	float				bat_volts;
} task_adc_dta_t;

/********************** external data declaration ****************************/
extern task_adc_dta_t task_adc_dta;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_TASK_ADC_ATTRIBUTE_H_ */
/********************** end of file ******************************************/
