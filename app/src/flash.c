/*
 * flash.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Gonzalo
 */
#include "flash.h"

void Flash_Write_Setup(flash_setup_t *setup) {
    // 1. Desbloquear la Flash
    HAL_FLASH_Unlock();

    // 2. Configurar el borrado de la página
    FLASH_EraseInitTypeDef eraseConfig;
    uint32_t pageError;

    eraseConfig.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseConfig.PageAddress = FLASH_USER_START_ADDR;
    eraseConfig.NbPages = 1; // Borramos solo nuestra página de configuración

    if (HAL_FLASHEx_Erase(&eraseConfig, &pageError) == HAL_OK) {
        // 3. Programar la estructura palabra por palabra (32 bits)
        uint32_t *dataPtr = (uint32_t *)setup;
        uint32_t currentAddr = FLASH_USER_START_ADDR;

        for (uint32_t i = 0; i < (sizeof(flash_setup_t) / 4); i++) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currentAddr, dataPtr[i]);
            currentAddr += 4; // Avanzamos 4 bytes (un Word)
        }
    }

    // 4. Bloquear la Flash por seguridad
    HAL_FLASH_Lock();
}

void Flash_Read_Setup(flash_setup_t *setup) {
    // La lectura es directa mediante mapeo de memoria
    *setup = *(volatile flash_setup_t *)FLASH_USER_START_ADDR;
}

