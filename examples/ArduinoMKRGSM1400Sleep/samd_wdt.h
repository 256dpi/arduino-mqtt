#include "inttypes.h"

// The generic clock provider for the WDT
// Arduino Zero default GCLK configs
// see cores/startup.c for details
// GCLK0   DFLL48M DIV1 = 48mhz
// GCLK1   XOSC32K DIV1 = 32khz
// GCLK2   
// GCLK3   OSC8M   DIV1 = 8mhz
#define WDT_GCLK    4

enum wdt_period : uint8_t 
{
  // See Arduino15/packages/arduino/tools/CMSIS/4.0.0-atmel/Device/ATMEL/samd21/include/component/wdt.h
  // It is easier to use numeric values as there are two
  // sets of macros one for the reset WDT_CONFIG_WINDOW()
  // and the other for the early warning period WDT_EWCTRL_EWOFFSET().
  // Clock is running at 1024hz
  WDT_PERIOD_1DIV64 = 1,   // 16 cycles   = 1/64s
  WDT_PERIOD_1DIV32 = 2,   // 32 cycles   = 1/32s
  WDT_PERIOD_1DIV16 = 3,   // 64 cycles   = 1/16s
  WDT_PERIOD_1DIV8  = 4,   // 128 cycles  = 1/8s
  WDT_PERIOD_1DIV4  = 5,   // 256 cycles  = 1/4s
  WDT_PERIOD_1DIV2  = 6,   // 512 cycles  = 1/2s
  WDT_PERIOD_1X     = 7,   // 1024 cycles = 1s
  WDT_PERIOD_2X     = 8,   // 2048 cycles = 2s
  WDT_PERIOD_4X     = 9,   // 4096 cycles = 4s
  WDT_PERIOD_8X     = 10   // 8192 cycles = 8s
};

volatile bool samdWdtFlag;

void enableWdt(wdt_period period) {
    // Here we use normal mode with  the early warning interrupt
  // enabled. The early warning period is defined by the parameter
  // 'period' and the reset is set to twice that value.

  // Turn the power to the WDT module on
  PM->APBAMASK.reg |= PM_APBAMASK_WDT;

  // We cannot configure the WDT if it is already in always on mode
  if (!(WDT->CTRL.reg & WDT_CTRL_ALWAYSON)) {
    
    // Setup clock provider WDT_GCLK with a 32 source divider
    // GCLK_GENDIV_ID(X) specifies which GCLK we are configuring
    // GCLK_GENDIV_DIV(Y) specifies the clock prescalar / divider
    // If GENCTRL.DIVSEL is set (see further below) the divider 
    // is 2^(Y+1). If GENCTRL.DIVSEL is 0, the divider is simply Y
    // This register has to be written in a single operation
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(WDT_GCLK) | 
                       GCLK_GENDIV_DIV(4);

    // Configure the GCLK module
    // GCLK_GENCTRL_GENEN, enable the specific GCLK module
    // GCLK_GENCTRL_SRC_OSCULP32K, set the source to the OSCULP32K
    // GCLK_GENCTRL_ID(X), specifies which GCLK we are configuring
    // GCLK_GENCTRL_DIVSEL, specify which prescalar mode we are using
    // Output from this module is 1khz (32khz / 32)
    // This register has to be written in a single operation.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | 
                        GCLK_GENCTRL_SRC_OSCULP32K | 
                        GCLK_GENCTRL_ID(WDT_GCLK) | 
                        GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
    
    // Configure the WDT clock
    // GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_WDT), specify the WDT clock
    // GCLK_CLKCTRL_GEN(WDT_GCLK), specify the source from the WDT_GCLK GCLK
    // This register has to be written in a single operation
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_WDT) |
                        GCLK_CLKCTRL_GEN(WDT_GCLK) | 
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    // Disable the module before configuring
    WDT->CTRL.reg &= ~WDT_CTRL_ENABLE;
    while (WDT->STATUS.reg & WDT_STATUS_SYNCBUSY);
    
    // Disable windowed mode
    WDT->CTRL.reg &= ~WDT_CTRL_WEN;
    while (WDT->STATUS.reg & WDT_STATUS_SYNCBUSY);

    // Set the reset period to twice that of the
    // specified interrupt period
    WDT->CONFIG.reg = WDT_CONFIG_PER(period + 1);

    // Set the early warning as specified by the period
    WDT->EWCTRL.reg = WDT_EWCTRL_EWOFFSET(period);

    // Enable the WDT module
    WDT->CTRL.reg |= WDT_CTRL_ENABLE;
    while (WDT->STATUS.reg & WDT_STATUS_SYNCBUSY);
  
    // Enable early warning interrupt
    WDT->INTENSET.reg = WDT_INTENSET_EW;
    
    // Enable interrupt vector for WDT
    // Priority is set to 0x00, the highest
    NVIC_EnableIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0x00);
  }
}

void disableWdt() {
  // Disable the WDT module
  WDT->CTRL.reg &= ~WDT_CTRL_ENABLE;
  while (WDT->STATUS.reg & WDT_STATUS_SYNCBUSY);
  
  // Turn off the power to the WDT module
  PM->APBAMASK.reg &= ~PM_APBAMASK_WDT;
}

void resetWdt() {
// Reset counter and wait for synchronisation
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
  while (WDT->STATUS.reg & WDT_STATUS_SYNCBUSY);
}

void WDT_Handler() {
  samdWdtFlag = true;
  
  // Clear the early warning interrupt flag
  WDT->INTFLAG.reg = WDT_INTFLAG_EW;
}

