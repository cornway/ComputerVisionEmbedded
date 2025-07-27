
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#include "uart.hpp"

LOG_MODULE_REGISTER(uart);

#define RECEIVE_TIMEOUT 10000

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_usart2)

UartEx::UartEx(uint8_t *buf, size_t size)
    : mUartDev(DEVICE_DT_GET(UART_DEVICE_NODE)) {

  ring_buf_init(&mRingBuffer, size, buf);
  mUartCfg = {.baudrate = DT_PROP(UART_DEVICE_NODE, current_speed),
              .parity = UART_CFG_PARITY_NONE,
              .stop_bits = UART_CFG_STOP_BITS_1,
              .data_bits = UART_CFG_DATA_BITS_8,
              .flow_ctrl = UART_CFG_FLOW_CTRL_NONE};
}

int UartEx::init() {
  if (!mUartDev) {
    printk("Failed to get UART device");
    return -ENOSYS;
  }

  /* uart configuration parameters */
  int err = uart_configure(mUartDev, &mUartCfg);

  if (err == -ENOSYS) {
    printk("Configuration is not supported by device or driver,"
           " check the UART settings configuration\n");
    return -ENOSYS;
  }

  /* Configure uart callback */
  uart_callback_set(mUartDev, &UartEx::callback, this);

  /* enable uart reception */
  uart_rx_enable(mUartDev, mUartRxBuffer, sizeof(mUartRxBuffer),
                 RECEIVE_TIMEOUT);

  return 0;
}

int UartEx::trigger() {
  uint8_t req = 0xff;
  int err = uart_tx(mUartDev, &req, 1, SYS_FOREVER_US);
  if (err) {
    LOG_ERR("uart_tx failed: %d", err);
  }
  return err;
}

size_t UartEx::tryRead(uint8_t *buf, size_t size) {
  uint32_t offset = 0;
  while (size != offset) {
    int len = ring_buf_get(&mRingBuffer, buf + offset, size - offset);

    offset += len;
    if (offset == size) {
      break;
    }

    k_sleep(K_MSEC(1));
  }

  return size;
}

size_t UartEx::tryReadHeader() {

  uint8_t buf[2] = {0};
  uint16_t *length = (uint16_t *)buf;
  int len = ring_buf_get(&mRingBuffer, buf, sizeof(buf));
  int offset = len;
  if (0 == len) {
    return 0;
  }
  while (true) {
    int len = ring_buf_get(&mRingBuffer, buf + offset, sizeof(buf) - offset);
    offset += len;
    if (offset == sizeof(buf)) {
      break;
    }
    k_sleep(K_MSEC(1));
  }
  // printf("buf = %x %x\n", buf[0], buf[1]);
  return (uint32_t)length[0];
}

void UartEx::callback(const struct device *dev, struct uart_event *evt,
                      void *userData)

{
  UartEx *uartEx = static_cast<UartEx *>(userData);
  // LOG_ERR("UART evt %d", evt->type);
  // LOG_ERR("offset=%u len=%u", evt->data.rx.offset, evt->data.rx.len);

  switch (evt->type) {
  case UART_RX_RDY:
    /* Data received; place into ring buffer */
    if (evt->data.rx.len) {
      ring_buf_put(&uartEx->mRingBuffer, evt->data.rx.buf + evt->data.rx.offset,
                   evt->data.rx.len);
    }
    break;

  case UART_RX_DISABLED:
    /* Re-enable RX */
    uart_rx_enable(uartEx->mUartDev, uartEx->mUartRxBuffer,
                   sizeof(uartEx->mUartRxBuffer), uartEx->UartReceiveTimeoutUs);

    break;

  default:
    break;
  }
}