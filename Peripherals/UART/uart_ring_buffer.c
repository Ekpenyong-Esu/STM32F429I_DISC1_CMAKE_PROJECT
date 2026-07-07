/**
 * @file uart_ring_buffer.c
 * @brief Ring buffer implementation for UART DMA
 */

#include "uart_ring_buffer.h"
#include <string.h>


RingBuffer_t rxRingBuffer;  // Remove static to make it accessible externally

void RingBuffer_Init(RingBuffer_t *ringBuffer)
{
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
    ringBuffer->count = 0;
    memset(ringBuffer->buffer, 0, sizeof(ringBuffer->buffer));
    DEBUG_PRINT("Ring buffer initialized");
}

bool RingBuffer_Put(RingBuffer_t *ringBuffer, uint8_t data)
{
    if (ringBuffer == NULL || RingBuffer_IsFull(ringBuffer)) {
        DEBUG_PRINT("Buffer Full");
        return false;
    }

    ringBuffer->buffer[ringBuffer->head] = data;
    ringBuffer->head = (ringBuffer->head + 1) % RING_BUFFER_SIZE;
    ringBuffer->count++;
    return true;
}

bool RingBuffer_Get(RingBuffer_t *ringBuffer, uint8_t *data)
{
    if (ringBuffer == NULL || data == NULL || RingBuffer_IsEmpty(ringBuffer)) {
        DEBUG_PRINT("Buffer Empty or NULL data pointer");
        return false;
    }

    *data = ringBuffer->buffer[ringBuffer->tail];
    ringBuffer->tail = (ringBuffer->tail + 1) % RING_BUFFER_SIZE;
    ringBuffer->count--;
    return true;
}

uint32_t RingBuffer_Available(RingBuffer_t *ringBuffer)
{
    return ringBuffer->count;
}

bool RingBuffer_IsFull(RingBuffer_t *ringBuffer)
{
    return ringBuffer->count >= RING_BUFFER_SIZE;
}

bool RingBuffer_IsEmpty(RingBuffer_t *ringBuffer)
{
    return ringBuffer->count == 0;
}

void UART_RingBuffer_Init(void)
{
    RingBuffer_Init(&rxRingBuffer);
}

UART_Status_t UART_RingBuffer_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size)
{
    if (handle == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("UART handle, data is NULL or size is 0");
        return UART_ERROR;
    }

    for (uint16_t i = 0; i < size; i++) {
        if (!RingBuffer_Get(&rxRingBuffer, &data[i])) {
            DEBUG_PRINT("Failed to get data from ring buffer");
            return UART_ERROR; // Buffer underflow
        }
    }

    return UART_OK;
}

void UART_RingBuffer_PutData(uint8_t* data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        RingBuffer_Put(&rxRingBuffer, data[i]);
    }
}
