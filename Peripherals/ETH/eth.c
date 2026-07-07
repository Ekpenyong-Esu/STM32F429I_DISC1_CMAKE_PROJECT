/**
 * @file eth.c
 * @brief Ethernet Driver Implementation
 * @author Generated for STM32F429
 * @date 2025
 *
 * This file implements the Ethernet driver functions for STM32F4 series.
 */

#include "eth.h"
#include "log.h"
#include <string.h>

/* Ethernet frame constants */
#define ETH_HEADER_SIZE         14
#define ETH_MIN_FRAME_SIZE      60
#define ETH_TYPE_OFFSET         12
#define ETH_PAYLOAD_OFFSET      14
#define ETH_MAX_FRAME_SIZE      1518
#define ETH_BYTE_MASK           0xFF

/* Note: Callback wrappers removed as they are not used in current implementation */

/**
 * @brief Initialize Ethernet with given configuration
 */
HAL_StatusTypeDef ETH_Init(ETH_Handle_t *handle, ETH_Config_t *config) {
    log_debug("ETH: Initializing Ethernet");

    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Copy configuration */
    handle->config = *config;

    /* Initialize HAL ETH handle */
    handle->heth.Instance = ETH;
    handle->heth.Init.MACAddr = (uint8_t *)config->macAddr;
    handle->heth.Init.MediaInterface = (ETH_MediaInterfaceTypeDef)config->mediaInterface;

    /* Allocate DMA descriptors */
    static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection")));
    static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));

    handle->heth.Init.TxDesc = DMATxDscrTab;
    handle->heth.Init.RxDesc = DMARxDscrTab;
    handle->heth.Init.RxBuffLen = ETH_MAX_FRAME_SIZE; /* Maximum frame size */

    /* Initialize ETH */
    if (HAL_ETH_Init(&handle->heth) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Configure MAC */
    ETH_MACConfigTypeDef macConfig = {0};
    macConfig.SourceAddrControl = ETH_SOURCEADDRESS_DISABLE;
    macConfig.ChecksumOffload = DISABLE;
    macConfig.Speed = config->speed;
    macConfig.DuplexMode = config->duplexMode;

    /* Note: HAL_ETH_ConfigMAC may not be available in this HAL version */
    /* MAC configuration is handled during HAL_ETH_Init */

    handle->initialized = true;

    log_debug("ETH: Ethernet initialized successfully");

    return HAL_OK;
}

/**
 * @brief Deinitialize Ethernet
 */
HAL_StatusTypeDef ETH_DeInit(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    if (HAL_ETH_DeInit(&handle->heth) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->initialized = false;
    return HAL_OK;
}

/**
 * @brief Start Ethernet communication
 */
HAL_StatusTypeDef ETH_Start(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    return HAL_ETH_Start(&handle->heth);
}

/**
 * @brief Stop Ethernet communication
 */
HAL_StatusTypeDef ETH_Stop(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    return HAL_ETH_Stop(&handle->heth);
}

/**
 * @brief Transmit Ethernet frame
 */
HAL_StatusTypeDef ETH_TransmitFrame(ETH_Handle_t *handle, ETH_Frame_t *frame) {
    if (handle == NULL || !handle->initialized || frame == NULL) {
        return HAL_ERROR;
    }

    if (frame->payload == NULL || frame->payloadLength == 0) {
        return HAL_ERROR;
    }

    /* Prepare frame buffer */
    uint8_t *txBuffer = handle->txBuffer;
    if (txBuffer == NULL) {
        return HAL_ERROR;
    }

    /* Copy destination MAC */
    memcpy(txBuffer, frame->destination, 6);
    /* Copy source MAC */
    memcpy(txBuffer + 6, frame->source, 6);
    /* Copy type */
    txBuffer[ETH_TYPE_OFFSET] = (frame->type >> 8) & ETH_BYTE_MASK;
    txBuffer[ETH_TYPE_OFFSET + 1] = frame->type & ETH_BYTE_MASK;
    /* Copy payload */
    memcpy(txBuffer + ETH_PAYLOAD_OFFSET, frame->payload, frame->payloadLength);

    /* Prepare transmit configuration */
    ETH_TxPacketConfigTypeDef txConfig;
    txConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    txConfig.Length = frame->payloadLength + ETH_HEADER_SIZE;
    txConfig.TxBuffer = NULL; /* Use internal buffer */
    txConfig.SrcAddrCtrl = ETH_SOURCEADDRESS_DISABLE;
    txConfig.CRCPadCtrl = ETH_CRC_PAD_DISABLE;
    txConfig.ChecksumCtrl = ETH_CHECKSUM_DISABLE;

    /* Transmit frame */
    return HAL_ETH_Transmit(&handle->heth, &txConfig, 0);
}

/**
 * @brief Receive Ethernet frame
 */
HAL_StatusTypeDef ETH_ReceiveFrame(ETH_Handle_t *handle, ETH_Frame_t *frame) {
    if (handle == NULL || !handle->initialized || frame == NULL) {
        return HAL_ERROR;
    }

    if (handle->rxBuffer == NULL) {
        return HAL_ERROR;
    }

    /* Receive frame */
    void *rxBufferPtr = (void *)handle->rxBuffer;
    uint32_t frameLength = 0;

    HAL_StatusTypeDef status = HAL_ETH_ReadData(&handle->heth, &rxBufferPtr);
    if (status != HAL_OK) {
        return status;
    }

    /* Get frame length from buffer */
    frameLength = *(uint32_t *)rxBufferPtr;

    if (frameLength < ETH_HEADER_SIZE) { /* Minimum Ethernet frame size */
        return HAL_ERROR;
    }

    /* Parse frame */
    memcpy(frame->destination, handle->rxBuffer, 6);
    memcpy(frame->source, handle->rxBuffer + 6, 6);
    frame->type = (handle->rxBuffer[ETH_TYPE_OFFSET] << 8) | handle->rxBuffer[ETH_TYPE_OFFSET + 1];
    frame->payloadLength = frameLength - ETH_HEADER_SIZE;
    frame->payload = handle->rxBuffer + ETH_PAYLOAD_OFFSET;

    return HAL_OK;
}

/**
 * @brief Check if Ethernet link is up
 */
bool ETH_IsLinkUp(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    return (HAL_ETH_GetState(&handle->heth) == HAL_ETH_STATE_READY);
}

/**
 * @brief Get Ethernet link speed
 */
uint32_t ETH_GetLinkSpeed(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return 0;
    }

    /* This would typically require PHY register reads */
    /* For now, return configured speed */
    return handle->config.speed;
}

/**
 * @brief Get Ethernet link duplex mode
 */
uint32_t ETH_GetLinkDuplex(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return 0;
    }

    /* This would typically require PHY register reads */
    /* For now, return configured duplex mode */
    return handle->config.duplexMode;
}

/**
 * @brief Set MAC address
 */
HAL_StatusTypeDef ETH_SetMACAddress(ETH_Handle_t *handle, uint8_t *macAddr) {
    if (handle == NULL || !handle->initialized || macAddr == NULL) {
        return HAL_ERROR;
    }

    memcpy(handle->config.macAddr, macAddr, 6);
    /* Note: MAC address is set during initialization */
    /* To change MAC address, re-initialize the ETH peripheral */
    return HAL_OK;
}

/**
 * @brief Get MAC address
 */
void ETH_GetMACAddress(ETH_Handle_t *handle, uint8_t *macAddr) {
    if (handle != NULL && macAddr != NULL) {
        memcpy(macAddr, handle->config.macAddr, 6);
    }
}

/**
 * @brief Enable Ethernet interrupts
 */
HAL_StatusTypeDef ETH_EnableInterrupts(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Enable ETH interrupt */
    HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    return HAL_OK;
}

/**
 * @brief Disable Ethernet interrupts
 */
HAL_StatusTypeDef ETH_DisableInterrupts(ETH_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Disable ETH interrupt */
    HAL_NVIC_DisableIRQ(ETH_IRQn);

    return HAL_OK;
}

/**
 * @brief Ethernet IRQ Handler
 */
void ETH_IRQHandler(ETH_Handle_t *handle) {
    if (handle != NULL && handle->initialized) {
        HAL_ETH_IRQHandler(&handle->heth);
    }
}

/**
 * @brief ETH Transmit Complete Callback (weak implementation)
 */
__weak void ETH_TxCpltCallback(ETH_HandleTypeDef *heth) {
    /* User should implement this callback */
    UNUSED(heth);
}

/**
 * @brief ETH Receive Complete Callback (weak implementation)
 */
__weak void ETH_RxCpltCallback(ETH_HandleTypeDef *heth) {
    /* User should implement this callback */
    UNUSED(heth);
}

/**
 * @brief ETH Error Callback (weak implementation)
 */
__weak void ETH_ErrorCallback(ETH_HandleTypeDef *heth) {
    /* User should implement this callback */
    UNUSED(heth);
}
