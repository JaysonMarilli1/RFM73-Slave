/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    /* TODO Initialize User Ports/Peripherals/Project here */
    // Initialises the chip
    OSCCON = 0x7f; // 8 MHz

    ANSELA = 0; // define port as digital
    ANSELB = 0; // define port as digital

    TRISA = 0x02; // TRIS: 0 = output, 1=input.
    TRISB = 0x22; // TRIS: 0 = output, 1=input

    WPUA = 0;   //Weak Pull up on A disabled
    WPUB = 0;   //Weak Pull up on B disabled

}

