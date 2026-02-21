/**
 * @file    task_system.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Declaraciones públicas (prototipos) de la tarea System.
 * @date    Jan 10, 2026
 */

#ifndef TASK_INC_TASK_SYSTEM_H_
#define TASK_INC_TASK_SYSTEM_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/
extern uint32_t g_task_system_cnt;
extern volatile uint32_t g_task_system_tick_cnt;

/********************** external functions declaration ***********************/
extern void task_system_init(void *parameters);
extern void task_system_update(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_SYSTEM_H_ */

/********************** end of file ******************************************/
