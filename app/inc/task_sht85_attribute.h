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
typedef enum task_sht85_ev {EV_SEN_IDLE,
							EV_SEN_MEASURE_ON,
						    EV_SEN_MEASURE_READ,
						    EV_SEN_FALLA_OK,} task_sht85_ev_t;

/* State of Task Menu */
typedef enum task_sht85_st {ST_SEN_IDLE,
						   ST_SEN_WAIT_TX,
						   ST_SEN_WAITING,
						   ST_SEN_READY,
						   ST_SEN_WAIT_RX,
						   ST_SEN_FALLA} task_sht85_st_t;

typedef struct
{
	uint32_t			tick;
	bool				flag;
	uint32_t			tick_means_max;
	uint32_t			tick_timeout_max;
} task_sht85_cfg_t;


typedef struct
{
	uint32_t			tick_measure;
	uint32_t 			tick_timeout;
	task_sht85_st_t		state;
	task_sht85_ev_t		event;

	bool				i2c_op_complete;
	bool				i2c_error;
	uint8_t				i2c_rx_raw_values[6];
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
