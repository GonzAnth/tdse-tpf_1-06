/*
 * flash.h
 *
 *  Created on: Jan 29, 2026
 *      Author: Gonzalo
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"

// Dirección de la última página de 1KB en un STM32F103RB (128KB total)
#define FLASH_USER_START_ADDR   0x0801FC00
#define FLASH_MAGIC_NUMBER      0xABCD1234

typedef struct {
    uint32_t magic_number;           // Identificador de datos válidos
    uint32_t threshold_temperature;  // Umbral de temperatura
    uint32_t threshold_humidity;     // Umbral de humedad
    uint32_t tick_idle_max;          // Tiempo de espera entre riegos
    uint32_t system_mode;            // Modo: Tiempo o Sensor
} flash_setup_t;

// Prototipos
void Flash_Write_Setup(flash_setup_t *setup);
void Flash_Read_Setup(flash_setup_t *setup);

#endif /* INC_FLASH_H_ */
