

#include "engr2350_msp432.h"

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#ifndef NULL
#define NULL 0
#endif

void init_std_uart();

void SysInit(){
    init_std_uart();
}
void init_std_uart(){
    char dev[] = "UART";
    add_device(dev, _MSA, dopen, dclose, dread, dwrite, dlseek, dunlink, drename);
    freopen("UART:0", "w", stdout);
    freopen("UART:0", "r", stdin);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
}

int dopen(const char *path, unsigned flags, int llv_fd){
    eUSCI_UART_ConfigV1 uart = {0};
    uart.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    uart.clockPrescalar = 13;
    uart.firstModReg = 0;
    uart.secondModReg = 0x25;
    uart.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
    uart.parity = EUSCI_A_UART_NO_PARITY;
    uart.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    uart.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    uart.uartMode = EUSCI_A_UART_MODE;
    uart.dataLength = EUSCI_A_UART_8_BIT_LEN;

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A0_BASE, &uart);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    return 0;
}

int dclose(int dev_fd){
    MAP_UART_disableModule(EUSCI_A0_BASE);
    return 0;
}

int dread(int dev_fd, char *buf, unsigned count){
    int i;
    for(i = 0; i < count; i++){
        buf[i] = MAP_UART_receiveData(EUSCI_A0_BASE);
        if(buf[i] == '\n') return count;
    }
    return count;
}

int dwrite(int dev_fd, const char *buf, unsigned count){
    int i;
    for(i = 0; i < count; i++) MAP_UART_transmitData(EUSCI_A0_BASE, buf[i]);
    return count;
}

long dlseek(int dev_fd, off_t offset, int origin){ return -1; }
int dunlink(const char *path){ return -1; }
int drename(const char *old_name, const char *new_name){ return -1; }

uint8_t getchar_nw(){
    // Implementation of a non-blocking getchar
    if((BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IFG, EUSCI_A_IFG_RXIFG_OFS)))
        return EUSCI_A_CMSIS(EUSCI_A0_BASE)->RXBUF;
    else
        return 0;
}
