//=====[#include guards - begin]===============================================
#ifndef SHT85_H_
#define SHT85_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>


//=====[Declaration of public defines]=========================================
// 0x44 << 1 = 0x88
#define SHT85_I2C_ADDR  (0x44 << 1)
#define SHT85_CMD_MEASURE_HIGH  0x2400
#define SHT85_CMD_MEASURE_LOW   0x2416


//=====[Declarations (prototypes) of public functions]=========================
void SHT85_Init(I2C_HandleTypeDef *hi2c);

bool SHT85_start_measure_IT(void);

bool SHT85_start_read_IT(uint8_t *buffer_destino);


bool SHT85_compute_values(uint8_t *rx_buffer, float* temp, float* hum);




//bool SHT85_send_single_shot(void);

//bool SHT85_read(float* temp, float* hum);




//=====[#include guards - end]=================================================

#endif /* SHT85_H_ */
