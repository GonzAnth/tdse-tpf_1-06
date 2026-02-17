/**
 * @file    sht85.c
 * @author  Gonzalo Antahuara & Dante Mele Ientile
 * @brief   Driver sensor SHT85.
 * @date    Dic 15, 2025
 */

#include "sht85.h"

static I2C_HandleTypeDef *phi2c;

//Polinomio CRC: x^8 + x^5 + x^4 + 1 = 0x31
#define CRC_POLYNOMIAL  0x31

//=====[Declarations (prototypes) of private functions]========================
static uint8_t CalculateCRC(uint8_t *data, uint8_t len);



//=====[Implementations of public functions]===================================
void SHT85_Init(I2C_HandleTypeDef *hi2c)
{
    phi2c = hi2c;
    HAL_Delay(2);
}


bool SHT85_start_measure_IT(void)
{
    static uint8_t cmd_buffer[2];
    uint16_t command = SHT85_CMD_MEASURE_HIGH;

    cmd_buffer[0] = (command >> 8) & 0xFF;
    cmd_buffer[1] = command & 0xFF;

    if (HAL_I2C_Master_Transmit_IT(phi2c, SHT85_I2C_ADDR, cmd_buffer, 2) != HAL_OK)
    {
        return false;
    }
    return true;
}


bool SHT85_start_read_IT(uint8_t *buffer_destino)
{
    if (HAL_I2C_Master_Receive_IT(phi2c, SHT85_I2C_ADDR, buffer_destino, 6) != HAL_OK)
    {
        return false;
    }
    return true;
}


bool SHT85_compute_values(uint8_t *rx_buffer, float* temp, float* hum)
{
    if (CalculateCRC(&rx_buffer[0], 2) != rx_buffer[2]) return false;
    if (CalculateCRC(&rx_buffer[3], 2) != rx_buffer[5]) return false;

    uint16_t raw_temp = (rx_buffer[0] << 8) | rx_buffer[1];
    uint16_t raw_hum = (rx_buffer[3] << 8) | rx_buffer[4];

    float t = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
    float h = 100.0f * ((float)raw_hum / 65535.0f);

    if ((raw_temp == 0) || (raw_hum == 0)) return false;

    *temp = t;
    *hum = h;

    return true;
}


static uint8_t CalculateCRC(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF; //Valor inicial
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC_POLYNOMIAL;
            else
                crc <<= 1;
        }
    }
    return crc;
}
