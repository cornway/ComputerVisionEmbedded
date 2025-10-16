

#pragma once

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>

class UartEx {
public:
  explicit UartEx(uint8_t *buf, size_t size);
  int init();
  int trigger();
  size_t tryReadHeader();
  size_t tryRead(uint8_t *buf, size_t size);

private:
  static constexpr size_t UartRxBufferSize = 64;
  static constexpr size_t UartReceiveTimeoutUs = 10000;

  static void callback(const struct device *dev, struct uart_event *evt,
                       void *userData);

  struct ring_buf mRingBuffer {};
  struct uart_config mUartCfg {};

  uint8_t mUartRxBuffer[UartRxBufferSize]{};
  const struct device *const mUartDev;
};