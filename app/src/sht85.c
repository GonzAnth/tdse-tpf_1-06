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




bool SHT85_send_single_shot(void)
{
    uint8_t cmd_buffer[2];
    uint16_t command = SHT85_CMD_MEASURE_LOW;

    cmd_buffer[0] = (command >> 8) & 0xFF;
    cmd_buffer[1] = command & 0xFF;

    if (HAL_I2C_Master_Transmit(phi2c, SHT85_I2C_ADDR, cmd_buffer, 2, 100) != HAL_OK)
    {
        return false;
    }

    return true;
}




bool SHT85_read(float* temp, float* hum)
{
    uint8_t rx_buffer[6]; // TempMSB, TempLSB, CRC, HumMSB, HumLSB, CRC
    if (HAL_I2C_Master_Receive(phi2c, SHT85_I2C_ADDR, rx_buffer, 6, 100) != HAL_OK)
    {
        return false;
    }


    //Verificamos CRC
    if (CalculateCRC(&rx_buffer[0], 2) != rx_buffer[2])
    {
        return false;
    }
    if (CalculateCRC(&rx_buffer[3], 2) != rx_buffer[5])
    {
        return false;
    }


    //Conversión
    uint16_t rawTemp = (rx_buffer[0] << 8) | rx_buffer[1];
    uint16_t rawHum = (rx_buffer[3] << 8) | rx_buffer[4];

    *temp = -45.0f + 175.0f * ((float)rawTemp / 65535.0f);
    *hum = 100.0f * ((float)rawHum / 65535.0f);

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
