/**
 * @file    flash.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Driver memoria flash.
 * @date    Jan 28, 2026
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
            // Calculamos cuántos "words" de 32 bits hay que escribir
            // El casting se hace de forma segura usando un puntero genérico
            uint32_t *dataPtr = (uint32_t *)setup;
            uint32_t wordsToWrite = sizeof(flash_setup_t) / 4;

            // Si por alguna razón el tamaño no fuera múltiplo de 4, redondeamos hacia arriba
            if (sizeof(flash_setup_t) % 4 != 0) wordsToWrite++;

            uint32_t currentAddr = FLASH_USER_START_ADDR;

            for (uint32_t i = 0; i < wordsToWrite; i++) {
                // Grabamos de a 32 bits
                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currentAddr, dataPtr[i]) != HAL_OK) {
                    // Opcional: Error handling aquí
                    break;
                }
                currentAddr += 4;
            }
        }

        HAL_FLASH_Lock();
    }



void Flash_Read_Setup(flash_setup_t *setup) {
    *setup = *(volatile flash_setup_t *)FLASH_USER_START_ADDR;
}

