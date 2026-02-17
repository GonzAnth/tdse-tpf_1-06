/**
 * @file    task_menu.h
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Declaraciones públicas (prototipos) de la tarea Menu.
 * @date    Jan 28, 2026
 */

#ifndef TASK_INC_TASK_MENU_H_
#define TASK_INC_TASK_MENU_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/
extern uint32_t g_task_menu_cnt;
extern volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions declaration ***********************/
extern void task_menu_init(void *parameters);
extern void task_menu_update(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_MENU_H_ */

/********************** end of file ******************************************/
