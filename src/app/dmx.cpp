#include "dmx.h"
#include "stm32f2xx_usart.h"
#include "stm32f2xx_rcc.h"

#define PIN_DRV_EN B0
#define PIN_RCV_EN B2

#define ENABLE true
#define DISABLE false

static const uint8_t CHAR_BREAK = 0;
static const uint8_t START_CODE_NULL = 0;
static const uint32_t BAUD_BREAK = 80000;
static const uint32_t BAUD_DMX = 250000;

namespace dmx {
  void receiverControl(bool enable);
  void driverControl(bool enable);

  void configureBaudRate(USART_TypeDef* USARTx, uint32_t baudRate) {
    RCC_ClocksTypeDef RCC_ClocksStatus;

    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;

    RCC_GetClocksFreq(&RCC_ClocksStatus);

    if ((USARTx == USART1) || (USARTx == USART6)) {
      apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    } else {
      apbclock = RCC_ClocksStatus.PCLK1_Frequency;
    }

    /* Determine the integer part */
    if ((USARTx->CR1 & USART_CR1_OVER8) != 0) {
      /* Integer part computing in case Oversampling mode is 8 Samples */
      integerdivider = ((25 * apbclock) / (2 * (baudRate)));
    } else { /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
      /* Integer part computing in case Oversampling mode is 16 Samples */
      integerdivider = ((25 * apbclock) / (4 * (baudRate)));
    }
    tmpreg = (integerdivider / 100) << 4;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

    /* Implement the fractional part in the register */
    if ((USARTx->CR1 & USART_CR1_OVER8) != 0) {
      tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
    } else {/* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
      tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
    }

    /* Write to USART BRR register */
    USARTx->BRR = (uint16_t)tmpreg;
  }

  void receiverControl(bool enable) {
    if (enable) {
      pinResetFast(PIN_RCV_EN); //active low
    } else {
      pinSetFast(PIN_RCV_EN);
    }
  }

  void driverControl(bool enable) {
    if (enable) {
      pinSetFast(PIN_DRV_EN);
    } else {
      pinResetFast(PIN_DRV_EN);
    }
  }

  void setup() {
    pinMode(PIN_DRV_EN, OUTPUT);
    pinMode(PIN_RCV_EN, OUTPUT);

    //disable both driver and receiver
    receiverControl(DISABLE);

    Serial1.begin(BAUD_DMX, SERIAL_8N2);
  }

  void send(const uint8_t *data, const uint32_t len) {
    driverControl(ENABLE);

    //send the break and mark after break for DMX by sending a break at a slower
    //baud rate.
    configureBaudRate(USART1, BAUD_BREAK);
    Serial1.write(CHAR_BREAK);
    Serial1.flush();

    configureBaudRate(USART1, BAUD_DMX);
    Serial1.write(START_CODE_NULL);

    Serial1.write(data, len);
    Serial1.flush();

    driverControl(DISABLE);
  }
}
