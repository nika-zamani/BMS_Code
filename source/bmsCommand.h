#ifndef BMSCOMMAND_H
#define BMSCOMMAND_H

#include "common.h"

enum Command
{
    WRCFGA,
    WRCFGB,
    RDCFGA,
    RDCFGB,
    RDCVA,
    RDCVB,
    RDCVC,
    RDCVD,
    RDCVE,
    RDCVF,
    RDAUXA,
    RDAUXB,
    RDAUXC,
    RDAUXD,
    RDSTATA,
    RDSTATB,
    WRSCTRL,
    WRPWM,
    WRPSB,
    RDSCTRL,
    RDPWM,
    RDPSB,
    STSCTRL,
    CLRSCTRL,
    ADCV,
    ADOW,
    CVST,
    ADOL,
    ADAX,
    ADAXD,
    AXST,
    ADSTAT,
    ADSTATD,
    STATST,
    ADCVAX,
    ADCVSC,
    CLRCELL,
    CLRAUX,
    CLRSTAT,
    PLADC,
    DIAGN,
    WRCOMM,
    RDCOMM,
    STCOMM
};

struct bmscommand_t; // forward declare struct for use in buildbuffer_f def

typedef int (*buildbuffer_f)(bmscommand_t *, uint8_t *); // assemble command buffer

// this struct used only in const definition of bmscom
typedef struct
{
    Command name;
    uint8_t code[2];
    buildbuffer_f combb;
} bmscom_t;

// COMmand Build Buffer
// the different possible buffer assembly procedures
int combbTx(bmscommand_t *, uint8_t *);  // send data
int combbRx(bmscommand_t *, uint8_t *);  // receive data
int combbDir(bmscommand_t *, uint8_t *); // send just instruction
int combbClk(bmscommand_t *, uint8_t *); // send clock pulses

const bmscom_t bmscom[44]{
    {WRCFGA, {0x00, 0x01}, combbTx},
    {WRCFGB, {0x00, 0x24}, combbTx},
    {RDCFGA, {0x00, 0x02}, combbRx},
    {RDCFGB, {0x00, 0x26}, combbRx},
    {RDCVA, {0x00, 0x04}, combbRx},
    {RDCVB, {0x00, 0x06}, combbRx},
    {RDCVC, {0x00, 0x08}, combbRx},
    {RDCVD, {0x00, 0x0A}, combbRx},
    {RDCVE, {0x00, 0x09}, combbRx},
    {RDCVF, {0x00, 0x0B}, combbRx},
    {RDAUXA, {0x00, 0x0C}, combbRx},
    {RDAUXB, {0x00, 0x0E}, combbRx},
    {RDAUXC, {0x00, 0x0D}, combbRx},
    {RDAUXD, {0x00, 0x0F}, combbRx},
    {RDSTATA, {0x00, 0x10}, combbRx},
    {RDSTATB, {0x00, 0x12}, combbRx},
    {WRSCTRL, {0x00, 0x14}, combbTx}, // TODO: all NULL combb defs
    {WRPWM, {0x00, 0x20}, NULL},
    {WRPSB, {0x00, 0x1C}, NULL},
    {RDSCTRL, {0x00, 0x16}, combbRx},
    {RDPWM, {0x00, 0x22}, combbRx},
    {RDPSB, {0x00, 0x1E}, combbRx},
    {STSCTRL, {0x00, 0x19}, combbDir},
    {CLRSCTRL, {0x00, 0x18}, combbDir},
    {ADCV, {0x02, 0x60}, combbDir},
    {ADOW, {0x02, 0x28}, combbDir},
    {CVST, {0x02, 0x07}, combbDir},
    {ADOL, {0x02, 0x01}, combbDir},
    {ADAX, {0x04, 0x60}, combbDir},
    {ADAXD, {0x04, 0x00}, combbDir},
    {AXST, {0x04, 0x07}, combbDir},
    {ADSTAT, {0x04, 0x68}, combbDir},
    {ADSTATD, {0x04, 0x08}, combbDir},
    {STATST, {0x04, 0x0F}, combbDir},
    {ADCVAX, {0x04, 0x6F}, combbDir},
    {ADCVSC, {0x04, 0x67}, combbDir},
    {CLRCELL, {0x07, 0x11}, NULL},
    {CLRAUX, {0x07, 0x12}, NULL},
    {CLRSTAT, {0x07, 0x13}, NULL},
    {PLADC, {0x07, 0x14}, NULL},
    {DIAGN, {0x07, 0x15}, NULL},
    {WRCOMM, {0x07, 0x21}, combbTx},
    {RDCOMM, {0x07, 0x22}, combbRx},
    {STCOMM, {0x07, 0x23}, combbClk},
};

// a command to be sent to the BMS
typedef struct bmscommand_t
{
    bmscom_t c;
    int num;        // number of chips
    int len;        // data length
    uint8_t *data;  // incoming/outgoing data
    uint8_t *error; // success flag
} bmscommand_t;

/*  Initializes data into a bms command
 *      @param c: The command to initialize
 *      @param com: The command definition
 *      @param num: The number of chips
 *      @param data: The data for the command
 *      @param result: Where to store the received data
 *      @param semaphore: semaphore which gets given by... something TODO: finish thought
 *      @param sent: pointer to success flag
 */
void bmsCommandInit(bmscommand_t *c, bmscom_t com, int num, uint8_t *data,
                    uint8_t *error);

/* buildCommandBuffer: maps an integer command id to its corresponding command
 * and loads said command's SPI command into the spi array.
 *
 *      @param command: A ponter to the command to turn into a buffer
 *      @param tx: A pointer to the spi buffer array [must be at least
 *              bmsCommandSize(command) bytes large]
 *
 *      @return An integer error code or 1 if successfull
 */
int buildCommandBuffer(bmscommand_t *command, uint8_t *tx);

/*  Gets the size of a command buffer [byte array] for this command
 *      @param c: Pointer to the command to get the size of
 *
 *      @return The size of the command as a byte array
 */
int bmsCommandSize(bmscommand_t *c);

/* Calculates and returns the PEC15 */
void pec15_calc(uint8_t len,   // Number of bytes to be used to calculate a PEC
                uint8_t *data, // Array of data to be used to calculate  a PEC
                uint8_t *pec); // Location of PEC

// Copied from LTSketchbook/libraries/LTC681x/LTC681x.cpp
const uint16_t crc15Table[256] = {0x0, 0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56,
                                  0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263,
                                  0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a,
                                  0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
                                  0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0,
                                  0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812,
                                  0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544, 0x2be, 0xc727, 0xcc15, 0x98c,
                                  0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a,
                                  0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d, 0x1a94,
                                  0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836,
                                  0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8,
                                  0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec,
                                  0x9975, 0x9247, 0x57de, 0x8423, 0x41ba, 0x4a88, 0x8f11, 0x57c, 0xc0e5,
                                  0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 0x34e2, 0x3fd0,
                                  0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
                                  0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb,
                                  0x4b42, 0x4070, 0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2,
                                  0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7,
                                  0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0,
                                  0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa,
                                  0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534,
                                  0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350, 0x51d6, 0x944f, 0x9f7d,
                                  0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3,
                                  0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e,
                                  0xe117, 0xea25, 0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510,
                                  0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453, 0x1ca, 0xd237, 0x17ae, 0x1c9c,
                                  0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02,
                                  0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368,
                                  0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095};

#endif
