/* Minimal RIA register stub for sim65 channel lifecycle tests. */
#ifndef TEST_RP6502_H
#define TEST_RP6502_H
#include <stdint.h>
typedef struct {
    uint16_t addr0;
    uint8_t step0;
    uint8_t rw0;
} TestRIA;
extern TestRIA test_ria;
#define RIA test_ria
int xreg(int device, int channel, int address, unsigned value);
#endif
