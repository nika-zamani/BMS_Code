/* datasheet page 35 */

/*
    The I/O ports GPIO3, GPIO4 and GPIO5 on LTC6811-1 and
    LTC6811-2 can be used as an I2C or SPI master port to
    communicate to an I2C or SPI slave. In the case of an I2C
    master, GPIO4 and GPIO5 form the SDA and SCL ports of
    the I2C interface respectively.
*/

/*
    LTC6811 has a 6-byte COMM register as shown in Table 18.
    This register stores all data and control bits required for
    I2C or SPI communication to a slave. The COMM register
    contains three bytes of data Dn[7:0] to be transmitted to
    or received from the slave device. ICOMn[3:0] specify
    control actions before transmitting/receiving each data
    byte. FCOMn[3:0] specify control actions after transmitting/
    receiving each data byte.
*/

/*
    If the bit ICOMn[3] in the COMM register is set to 1 the
    part becomes a SPI master and if the bit is set to 0 the
    part becomes an I2C master.
*/

/*
Table 18. COMM Register Memory Map
REGISTER  RD/WR BIT 7     BIT 6     BIT 5     BIT 4     BIT 3     BIT 2     BIT 1     BIT 0
COMM0     RD/WR ICOM0[3]  ICOM0[2]  ICOM0[1]  ICOM0[0]  D0[7]     D0[6]     D0[5]     D0[4]
COMM1     RD/WR D0[3]     D0[2]     D0[1]     D0[0]     FCOM0[3]  FCOM0[2]  FCOM0[1]  FCOM0[0]
COMM2     RD/WR ICOM1[3]  ICOM1[2]  ICOM1[1]  ICOM1[0]  D1[7]     D1[6]     D1[5]     D1[4]
COMM3     RD/WR D1[3]     D1[2]     D1[1]     D1[0]     FCOM1[3]  FCOM1[2]  FCOM1[1]  FCOM1[0]
COMM4     RD/WR ICOM2[3]  ICOM2[2]  ICOM2[1]  ICOM2[0]  D2[7]     D2[6]     D2[5]     D2[4]
COMM5     RD/WR D2[3]     D2[2]     D2[1]     D2[0]     FCOM2[3]  FCOM2[2]  FCOM2[1]  FCOM2[0]
*/

/*
Table 19. Write Codes for ICOMn[3:0] and FCOMn[3:0] on I2C Master
CONTROL BITS      | CODE  | ACTION            | DESCRIPTION
------------------------------------------------------------------------------------------------------------------
ICOMn[3:0]        | 0110  | START             | Generate a START signal on I2C port followed by data transmission
                  | 0001  | STOP              | Generate a STOP signal on I2C port
                  | 0000  | BLANK             | Proceed directly to data transmission on I2C port
                  | 0111  | No Transmit       | Release SDA and SCL and ignore the rest of the data
------------------------------------------------------------------------------------------------------------------
FCOMn[3:0]        | 0000  | Master ACK        | Master generates an ACK signal on ninth clock cycle
                  | 1000  | Master NACK       | Master generates a NACK signal on ninth clock cycle
                  | 1001  | Master NACK + STOP| Master generates a NACK signal followed by STOP signal
*/

/*
    Three commands help accomplish I2C or SPI communication
    to the slave device:

    WRCOMM Command: This command is used to write data
        to the COMM register. This command writes 6 bytes of
        data to the COMM register.

    STCOMM Command: This command initiates I2C/SPI communication
        on the GPIO ports. The COMM register contains
        3 bytes of data to be transmitted to the slave. During this
        command, the data bytes stored in the COMM register are
        transmitted to the slave I2C or SPI device and the data
        received from the I2C or SPI device is stored in the COMM
        register. This command uses GPIO4 (SDA) and GPIO5
        (SCL) for I2C communication

        The STCOMM command is to be followed by 24 clock
        cycles for each byte of data to be transmitted to the slave
        device while holding CSB low. For example, to transmit
        three bytes of data to the slave, send STCOMM command
        and its PEC followed by 72 clock cycles. Pull CSB high
        at the end of the 72 clock cycles of STCOMM command.
        During I2C or SPI communication, the data received from
        the slave device is updated in the COMM register.

    RDCOMM Command: The data received from the slave
        device can be read back from the COMM register using
        the RDCOMM command. The command reads back six
        bytes of data followed by the PEC. See the section Bus
        Protocols for more details on a read command format.
        Table 21 describes the possible read back codes for
        ICOMn[3:0] and FCOMn[3:0] when using the part as an
        I2C master. Dn[7:0] contains the data byte transmitted by
        the I2C slave.

    Any number of bytes can be transmitted to the slave in
    groups of 3 bytes using these commands. The GPIO ports
    will not get reset between different STCOMM commands.
    However, if the wait time between the commands is greater
    than 2s, the watchdog will time out and reset the ports to
    their default values.
    To transmit several bytes of data using an I2C master, a
    START signal is only required at the beginning of the entire
    data stream. A STOP signal is only required at the end of
    the data stream. All intermediate data groups can use a
    BLANK code before the data byte and an ACK/NACK signal
    as appropriate after the data byte. SDA and SCL will not
    get reset between different STCOMM commands.
*/

/*
Table 21. Read Codes for ICOMn[3:0] and FCOMn[3:0] on I2C Master
CONTROL BITS    CODE    DESCRIPTION
ICOMn[3:0]      0110    Master generated a START signal
                0001    Master generated a STOP signal
                0000    Blank, SDA was held low between bytes
                0111    Blank, SDA was held high between bytes

FCOMn[3:0]      0000    Master generated an ACK signal
                0111    Slave generated an ACK signal
                1111    Slave generated a NACK signal
                0001    Slave generated an ACK signal, master generated a STOP signal
                1001    Slave generated a NACK signal, master generated a STOP signal
*/

/* Timing info on datasheet */

//Steps to sent NUM_BYTES amount of data
// TODO: also need BLANK and ACK/NACK codes for inner data

// 1) START SIGNAL

// 2) for i in range floor(NUM_BYTES/3)

//      WRCOMM next 3 bytes
//      STCOMM
//      wait 72 cycles

//  3) WRCOM next NUM_BYTES % 3 bytes of data
//     STCOMM
//     wait 24 * (NUM_BYTES % 3) cycles     

//  4) STOP SIGNAL
