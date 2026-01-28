/**
 * @file    task_adc_attribute.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Definiciones de estructuras de datos, estados y configuración del ADC.
 * @date    Jan 24, 2026
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
#define ADC_MIN_COUNT       		0.0f
#define TEMP_V25            		1.43f
#define TEMP_AVG_SLOPE      		0.0043f


/********************** typedef **********************************************/

/* Events to excite Task adc */
typedef enum task_adc_ev {EV_ADC_IDLE,
						  EV_ADC_START,
						  EV_ADC_FALLA_OK} task_adc_ev_t;

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
	task_adc_ev_t  		ev_sys_adc_ok; //eventos que genera el ADC para el systema
	task_adc_ev_t  		ev_sys_adc_not_ok; //eventos que genera el ADC para el systema
} task_adc_cfg_t;

typedef struct
{
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
