/**
 * @file    task_sht85.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Declaraciones públicas (prototipos) de la tarea sht85.
 * @date    Dic 15, 2025
 */

#ifndef TASK_INC_TASK_SHT85_H_
#define TASK_INC_TASK_SHT85_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/
extern uint32_t g_task_sht85_cnt;
extern volatile uint32_t g_task_sht85_tick_cnt;

/********************** external functions declaration ***********************/
extern void task_sht85_init(void *parameters);
extern void task_sht85_update(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_SHT85_H_ */

/********************** end of file ******************************************/
