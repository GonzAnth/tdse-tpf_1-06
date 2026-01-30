//=====[#include guards - begin]===============================================

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdint.h>

//=====[Declaration of public defines]=========================================
#define ANCHO_LCD 20

#define PART_LEFT   0
#define PART_RIGHT  1
//=====[Declaration of public data types]======================================

typedef enum {
     DISPLAY_CONNECTION_GPIO_4BITS,
     DISPLAY_CONNECTION_GPIO_8BITS,
} displayConnection_t;

typedef struct {
   displayConnection_t connection;
} display_t;

//=====[Declarations (prototypes) of public functions]=========================

void displayInit( displayConnection_t connection );

void displayCharPositionWrite( uint8_t charPositionX, uint8_t charPositionY );

void displayStringWrite( const char * str );

void displayWritePart(uint8_t row, uint8_t col, const char *str, uint8_t max_len);

void displayClearPart(uint8_t row, uint8_t col, uint8_t len);

void displayRowSplit(uint8_t row, uint8_t col_offset, const char *str, uint8_t part);

void displayUpdateRow(uint8_t row, uint8_t col, const char *str);

void displayClearRow(uint8_t row);


//=====[#include guards - end]=================================================

#endif // _DISPLAY_H_
