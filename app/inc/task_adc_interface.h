/*
 * task_adc_interface.h
 *
 *  Created on: Jan 24, 2026
 *      Author: Gonzalo
 */

#ifndef INC_TASK_ADC_INTERFACE_H_
#define INC_TASK_ADC_INTERFACE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include "task_adc_attribute.h"
/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
extern void init_queue_event_task_adc(void);
extern void put_event_task_adc(task_adc_ev_t event);
extern task_adc_ev_t get_event_task_adc(void);
extern bool any_event_task_adc(void);
void get_values_task_adc(float *temp, float *bat);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_TASK_ADC_INTERFACE_H_ */

/********************** end of file ******************************************/
