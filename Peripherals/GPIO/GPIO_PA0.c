#include "gpio.h"

void initialize_gpio_interrupts(void) {
    GPIO_Init();
    /* Configure user button pin for interrupt mode */
    GPIO_PA0_Button_Init(INTERRUPT_MODE);
}
