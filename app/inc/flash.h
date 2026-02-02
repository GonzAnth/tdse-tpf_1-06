/*
 * flash.h
 *
 *  Created on: Jan 29, 2026
 *      Author: Gonzalo
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"


#define FLASH_USER_START_ADDR   0x0800FC00

#define FLASH_MAGIC_NUMBER      0xABCD1234


typedef struct __attribute__((aligned(4))) {
    uint32_t 				magic_number;
    uint32_t 				tick_idle_max;
    uint32_t 				tick_riego_max;
    uint32_t 				threshold_temperature;
    uint32_t 				threshold_humidity;
} flash_setup_t;


// Prototipos
void Flash_Write_Setup(flash_setup_t *setup);
void Flash_Read_Setup(flash_setup_t *setup);

#endif /* INC_FLASH_H_ */
