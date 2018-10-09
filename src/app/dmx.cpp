#include "dmx.h"
#include "stm32f2xx_usart.h"
#include "stm32f2xx_rcc.h"

static const uint8_t CHAR_BREAK = 0;
static const uint8_t START_CODE_NULL = 0;
static const uint32_t BAUD_BREAK = 80000;
static const uint32_t BAUD_DMX = 250000;

void configureBaudRate(USART_TypeDef* USARTx, uint32_t baudRate) {
  RCC_ClocksTypeDef RCC_ClocksStatus;

  uint32_t tmpreg = 0x00, apbclock = 0x00;
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;

  RCC_GetClocksFreq(&RCC_ClocksStatus);

  if ((USARTx == USART1) || (USARTx == USART6))
  {
    apbclock = RCC_ClocksStatus.PCLK2_Frequency;
  }
  else
  {
    apbclock = RCC_ClocksStatus.PCLK1_Frequency;
  }

  /* Determine the integer part */
  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
  {
    /* Integer part computing in case Oversampling mode is 8 Samples */
    integerdivider = ((25 * apbclock) / (2 * (baudRate)));
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    /* Integer part computing in case Oversampling mode is 16 Samples */
    integerdivider = ((25 * apbclock) / (4 * (baudRate)));
  }
  tmpreg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }

  /* Write to USART BRR register */
  USARTx->BRR = (uint16_t)tmpreg;
}

int DEBUG_PIN = D0;

namespace dmx {
  void setup() {
    pinMode(DEBUG_PIN, OUTPUT);
    pinResetFast(DEBUG_PIN);

    Serial1.begin(BAUD_DMX, SERIAL_8N2);
  }

  void send(const uint8_t *data, const uint32_t len) {
    configureBaudRate(USART1, BAUD_BREAK);
    // Serial1.write(CHAR_BREAK);
    // Serial1.flush();
    USART_GetFlagStatus(USART1, USART_FLAG_TC);
    USART_SendData(USART1, CHAR_BREAK);

    pinSetFast(DEBUG_PIN);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {

    }
    pinResetFast(DEBUG_PIN);

    configureBaudRate(USART1, BAUD_DMX);
    // Serial1.write(START_CODE_NULL);
    pinSetFast(DEBUG_PIN);
    Serial1.write(data, len);
    Serial1.flush();
    pinResetFast(DEBUG_PIN);
  }
}