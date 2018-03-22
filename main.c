/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

//#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */

#include "RFM73.h"
//#include "rfm73.h"
//#include "rfm73-config.h"

#define _XTAL_FREQ 8000000

#define BUTTON_CANCEL 	RA0
#define BUTTON_CALL 	RA1
#define LED_RED 	RA2
#define LED_GREEN 	RA3
#define LED_YELLOW 	RA4
#define M_S_SELECT 	RA6
#define LED_CALL 	RA7

UINT8 count_50hz;

typedef struct {
    unsigned char reach_1s : 1;
    unsigned char reach_5hz : 1;
} FlagType;

FlagType Flag;

// PIC16F1827 Configuration Bit Settings
__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF &
        CPD_OFF & BOREN_OFF & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
__CONFIG(WRT_OFF & PLLEN_OFF & STVREN_OFF & LVP_OFF);

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

void init_mcu(void);
void init_port(void);
void timer2_init(void);

void power_on_delay(void);
void delay_200ms(void);
void delay_50ms(void);
void delay_5ms(void);
void delay_1ms(void);
void delay_20us(void);
void sub_program_1hz(char to_SEND);

void Send_Packet(UINT8 type, UINT8* pbuf, UINT8 len);
void Send_NACK_Packet(void);
void Receive_Packet(void);
void SPI_Bank1_Write_Reg(UINT8 reg, UINT8 *pBuf);
void SPI_Bank1_Read_Reg(UINT8 reg, UINT8 *pBuf);
void Carrier_Test(UINT8 b_enable); //carrier test

extern void RFM73_Initialize(void);
extern void SwitchToTxMode(void);
extern void SwitchToRxMode(void);

//const UINT8 tx_buf[17] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x78};
const UINT8 tx_buf_conf_CALL[17] = {0x30, 0x2b, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xdf};
const UINT8 tx_buf_conf_CANCEL[17] = {0x30, 0x23, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xd7};
const UINT8 tx_buf_test[17] = {0x30, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xe2};

UINT8 rx_buf[MAX_PACKET_LEN];

unsigned char buff1 = '.';

const char confirm_CALL = '1';
const char confirm_CANCEL = '0';
const char test_DATA = 'Z';

unsigned char call_FLAG = '1';
unsigned char cancel_FLAG = '1';

extern const UINT8 RX0_Address[];
extern const unsigned long Bank1_Reg0_13[];

UINT8 test_data;

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/
void main(void) {
    //unsigned char i, j, chksum;
    /* Initialize I/O and Peripherals for application */
    //InitApp();
    power_on_delay();
    init_mcu();
    count_50hz = 0;
    Flag.reach_1s = 0;

    INTCON = 0xc0; // enable interrupt
    RFM73_Initialize();

    //PORTA = 0;  //Set Port to correct state on startup
    //PORTB = 0;  //Set Port to correct state on startup

    LED_YELLOW = 1;
    call_FLAG = '1';
    cancel_FLAG = '1';
    
    while (1) {
        /* TODO <INSERT USER APPLICATION CODE HERE> */
        NOP();

        ////sub_program_1hz
        ////SwitchToRxMode();
        Receive_Packet();

        if (BUTTON_CALL == 0) {
            NOP();
            sub_program_1hz(test_DATA);
        }

        if (buff1 == '1' && call_FLAG == '1') {
            NOP();
            LED_CALL = 1;
            sub_program_1hz(confirm_CALL);
            call_FLAG = '0';
            cancel_FLAG = '1';
        }
        if (buff1 == '0' && cancel_FLAG == '1') {
            NOP();
            LED_CALL = 0;
            sub_program_1hz(confirm_CANCEL);
            call_FLAG = '1';
            cancel_FLAG = '0';
        }

    }

}

/*********************************************************
Function: init_mcu();

Description:
        initialize mcu.
 *********************************************************/
void init_mcu(void) {
    init_port();
    timer2_init();
}

/*********************************************************
Function: init_port();

Description:
        initialize port.
 *********************************************************/
void init_port(void) {
    OSCCON = 0x7f; // 8 MHz

    ANSELA = 0; // define port as digital
    ANSELB = 0; // define port as digital

    TRISA = 0x43; // TRIS: 0 = output, 1=input.
    TRISB = 0x22; // TRIS: 0 = output, 1=input

    WPUA = 0; //Weak Pull up on A disabled
    WPUB = 0; //Weak Pull up on B disabled

    CE_OUT();
    CSN_OUT();
    SCK_OUT();
    MISO_IN();
    MOSI_OUT();
    IRQ_IN();

    CE = 0;
    CSN = 1;
    SCK = 0;
    MOSI = 0;

    LED_CALL = 0;
    LED_RED = 0;
    LED_GREEN = 0;
    LED_YELLOW = 0;
}

/*********************************************************
Function: timer0_init();

Description:
        initialize timer.
 *********************************************************/
void timer2_init(void) {
    T2CON = 0x7e; // timer2 on and 16 pre, postscale
    PR2 = 156; // 50hZ, 4m/4/16/16/50
    TMR2IE = 1;
}

/*********************************************************
Function:  interrupt ISR_timer()

Description:

 *********************************************************/
void interrupt ISR_timer(void) {
    //	di();
    unsigned char i;
    if (TMR2IF) {   //if (TMR2IF) {

        count_50hz++;
        if (count_50hz == 50) // REACH 1S
        {
            count_50hz = 0;
            Flag.reach_1s = 1;

        }
        else if (count_50hz == 5) {
            Flag.reach_5hz = 1;
        }

        TMR2IF = 0;
    }
}

/*********************************************************
Function:      power_on_delay()

Description:

 *********************************************************/
void power_on_delay(void) {
    unsigned int i;
    for (i = 0; i < 1000; i++) {
        delay_1ms();
    }
}

/********************************************************

 *********************************************************/
void delay_200ms(void) {
    unsigned char j;
    for (j = 0; j < 40; j++) {
        delay_5ms();
    }
}

/*********************************************************
Function: delay_50ms();

Description:

 *********************************************************/
void delay_50ms(void) {
    unsigned char j;
    for (j = 0; j < 10; j++) {
        delay_5ms();
    }
}

/*********************************************************
Function: delay_5ms();

Description:

 *********************************************************/
void delay_5ms(void) {
    int i;
    for (i = 0; i < 650; i++) // 85*5
    {
        ;
    }
}

/*********************************************************
Function: delay_1ms();

Description:

 *********************************************************/
void delay_1ms(void) {
    unsigned char i;
    for (i = 0; i < 130; i++) {
        ;
    }
}

/*********************************************************
Function: delay_20us();

Description:

 *********************************************************/
void delay_20us(void) {
    unsigned char i;
    for (i = 0; i < 3; i++) {
        ;
    }
}

/*********************************************************
Function:  sub_program_1hz()

Description:

 *********************************************************/
void sub_program_1hz(char to_SEND) {
    UINT8 i;
    UINT8 temp_buf[32];

    if (Flag.reach_1s) {
        Flag.reach_1s = 0;

        if (to_SEND == '1') {
            for (i = 0; i < 17; i++) {
                temp_buf[i] = tx_buf_conf_CALL[i];
            }
        }
        if (to_SEND == '0') {
            for (i = 0; i < 17; i++) {
                temp_buf[i] = tx_buf_conf_CANCEL[i];
            }
        }
        if (to_SEND == 'Z') {
            for (i = 0; i < 17; i++) {
                temp_buf[i] = tx_buf_test[i];
            }
        }
        

        Send_Packet(W_TX_PAYLOAD_NOACK_CMD, temp_buf, 17);
        SwitchToRxMode(); //switch to Rx mode
    }
}

/**************************************************
Function: Send_Packet
Description:
        fill FIFO to send a packet
Parameter:
        type: WR_TX_PLOAD or  W_TX_PAYLOAD_NOACK_CMD
        pbuf: a buffer pointer
        len: packet length
Return:
        None
 **************************************************/
void Send_Packet(UINT8 type, UINT8* pbuf, UINT8 len) {
    UINT8 fifo_sta;

    SwitchToTxMode(); //switch to tx mode

    fifo_sta = SPI_Read_Reg(FIFO_STATUS); // read register FIFO_STATUS's value
    if ((fifo_sta & FIFO_STATUS_TX_FULL) == 0)//if not full, send data (write buff)
    {
        LED_RED = 1;

        SPI_Write_Buf(type, pbuf, len); // Writes data to buffer

        delay_50ms();
        LED_RED = 0;
        delay_50ms();
    }
}

/**************************************************
Function: Receive_Packet
Description:
        read FIFO to read a packet
Parameter:
        None
Return:
        None
 **************************************************/
void Receive_Packet(void) {
    UINT8 len, i, sta, fifo_sta, value, chksum, aa;
    UINT8 rx_buf[MAX_PACKET_LEN];

    sta = SPI_Read_Reg(STATUS); // read register STATUS's value

    if ((STATUS_RX_DR & sta) == 0x40) // if receive data ready (RX_DR) interrupt
    {
        do {
            len = SPI_Read_Reg(R_RX_PL_WID_CMD); // read len

            if (len <= MAX_PACKET_LEN) {
                SPI_Read_Buf(RD_RX_PLOAD, rx_buf, len); // read receive payload from RX_FIFO buffer
            } else {
                SPI_Write_Reg(FLUSH_RX, 0); //flush Rx
            }

            fifo_sta = SPI_Read_Reg(FIFO_STATUS); // read register FIFO_STATUS's value

        } while ((fifo_sta & FIFO_STATUS_RX_EMPTY) == 0); //while not empty

        chksum = 0;
        for (i = 0; i < 16; i++) {
            chksum += rx_buf[i];
        }
        if (chksum == rx_buf[16] && rx_buf[0] == 0x30) {
            LED_GREEN = 1;
            delay_50ms();
            delay_50ms();
            LED_GREEN = 0;

            
            buff1 = rx_buf[1];
            

            //Send_Packet(W_TX_PAYLOAD_NOACK_CMD,rx_buf,17);
            SwitchToRxMode(); //switch to RX mode
        }
    }
    SPI_Write_Reg(WRITE_REG | STATUS, sta); // clear RX_DR or TX_DS or MAX_RT interrupt flag
}

