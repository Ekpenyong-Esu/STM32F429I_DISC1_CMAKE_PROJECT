/**
 * @file uart_ring_buffer.h
 * @brief Ring buffer implementation for UART DMA
 */

#ifndef UART_RING_BUFFER_H
#define UART_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "uart.h"


#define RING_BUFFER_SIZE 512

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t count;
} RingBuffer_t;

extern RingBuffer_t rxRingBuffer;  // Make ring buffer accessible to other files

void RingBuffer_Init(RingBuffer_t *ringBuffer);
bool RingBuffer_Put(RingBuffer_t *ringBuffer, uint8_t data);
bool RingBuffer_Get(RingBuffer_t *ringBuffer, uint8_t *data);
uint32_t RingBuffer_Available(RingBuffer_t *ringBuffer);
bool RingBuffer_IsFull(RingBuffer_t *ringBuffer);
bool RingBuffer_IsEmpty(RingBuffer_t *ringBuffer);
UART_Status_t UART_RingBuffer_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size);
void UART_RingBuffer_PutData(uint8_t* data, uint16_t size);
void UART_RingBuffer_Init(void);

#endif // UART_RING_BUFFER_H
