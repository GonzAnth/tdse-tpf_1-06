/**
 * @file    task_actuator_attribute.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Definiciones de estructuras de datos, estados y configuración de los actuadores.
 * @date    Jan 24, 2026
 */

#ifndef TASK_INC_TASK_ACTUATOR_ATTRIBUTESH_
#define TASK_INC_TASK_ACTUATOR_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/* Events to excite Task Actuator */
typedef enum task_actuator_ev {EV_ACT_IDLE,
							   EV_ACT_OFF,
							   EV_ACT_ON,
							   EV_ACT_PULSE} task_actuator_ev_t;

/* States of Task Actuator */
typedef enum task_actuator_st {ST_ACT_ON,
							   ST_ACT_OFF,
							   ST_ACT_PULSE} task_actuator_st_t;

/* Identifier of Task Actuator */
typedef enum task_actuator_id {ID_ACT_RELAY,
							   ID_ACT_BUZZER} task_actuator_id_t;

typedef struct
{
	task_actuator_id_t	identifier;
	GPIO_TypeDef *		gpio_port;
	uint16_t			pin;
	GPIO_PinState		act_on;
	GPIO_PinState		act_off;
	uint32_t			tick_pulse_max;
} task_actuator_cfg_t;

typedef struct
{
	uint32_t			tick;
	uint32_t			tick_pulse;
	task_actuator_st_t	state;
	task_actuator_ev_t	event;
	bool				flag;
} task_actuator_dta_t;

/********************** external data declaration ****************************/
extern task_actuator_dta_t task_actuator_dta_list[];

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_ACTUATOR_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
