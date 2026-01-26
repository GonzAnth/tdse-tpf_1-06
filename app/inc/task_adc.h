/*
 * task_adc.h
 *
 *  Created on: Jan 24, 2026
 *      Author: Gonzalo
 */

#ifndef INC_TASK_ADC_H_
#define INC_TASK_ADC_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/
extern uint32_t g_task_adc_cnt;
extern volatile uint32_t g_task_adc_tick_cnt;

/********************** external functions declaration ***********************/
extern void task_adc_init(void *parameters);
extern void task_adc_update(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif


#endif /* INC_TASK_ADC_H_ */

/********************** end of file ******************************************/
