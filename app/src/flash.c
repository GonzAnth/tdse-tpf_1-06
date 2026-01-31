/**
 * @file    flash.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Driver memoria flash.
 * @date    Jan 28, 2026
 */

#include "flash.h"


void Flash_Write_Setup(flash_setup_t *setup) {
    HAL_FLASH_Unlock();

    // Borrado de página
    FLASH_EraseInitTypeDef eraseConfig;
    uint32_t pageError;
    eraseConfig.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseConfig.PageAddress = FLASH_USER_START_ADDR;
    eraseConfig.NbPages = 1;


    if (HAL_FLASHEx_Erase(&eraseConfig, &pageError) == HAL_OK) {
		uint32_t *dataPtr = (uint32_t *)setup;
		uint32_t wordsToWrite = sizeof(flash_setup_t) / 4;

		// Se redondea si no es múltiplo de 4
		if (sizeof(flash_setup_t) % 4 != 0) wordsToWrite++;

		uint32_t currentAddr = FLASH_USER_START_ADDR;
		for (uint32_t i = 0; i < wordsToWrite; i++) {
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currentAddr, dataPtr[i]) != HAL_OK) {
				//Error
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

