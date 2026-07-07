/**
 * @file eth.h
 * @brief Ethernet Driver Header File
 * @author Generated for STM32F429
 * @date 2025
 *
 * This Ethernet driver provides a reusable interface for Ethernet operations on STM32F4 series.
 * It wraps around STM32 HAL ETH functions for better abstraction and reusability.
 */

#ifndef ETH_H
#define ETH_H


#include "stm32f4xx.h"
#include "stm32f4xx_hal_eth.h"
#include <stdbool.h>
#include <stdint.h>

/* Ethernet Data Size Definitions */
#define ETH_DATA_SIZE_BYTE       0x00U
#define ETH_DATA_SIZE_HALFWORD   0x01U
#define ETH_DATA_SIZE_WORD       0x02U

/* Ethernet Protocol Types */
#define ETH_TYPE_ARP             0x0806U
#define ETH_TYPE_IPV4            0x0800U
#define ETH_TYPE_IPV6            0x86DDU

/* Ethernet Frame Constants */
#define ETH_ADDR_LEN             6U
#define ETH_HEADER_LEN           14U
#define ETH_MIN_FRAME_LEN        60U
#define ETH_MAX_FRAME_LEN        1518U

/* Ethernet Configuration Structure */
typedef struct {
    uint8_t macAddr[6];              /* MAC address */
    uint32_t speed;                  /* ETH_SPEED_10M or ETH_SPEED_100M */
    uint32_t duplexMode;             /* ETH_MODE_FULLDUPLEX or ETH_MODE_HALFDUPLEX */
    uint32_t checksumMode;           /* ETH_CHECKSUM_BY_HARDWARE or ETH_CHECKSUM_BY_SOFTWARE */
    uint32_t mediaInterface;         /* ETH_MEDIA_INTERFACE_RMII or ETH_MEDIA_INTERFACE_MII */
    uint32_t vlanTagIdentifier;      /* VLAN tag identifier */
    uint32_t vlanTagProtocol;        /* VLAN tag protocol */
} ETH_Config_t;

/* Ethernet Handle Structure */
typedef struct {
    ETH_HandleTypeDef heth;          /* HAL ETH handle */
    ETH_Config_t config;             /* Configuration */
    bool initialized;                /* Initialization status */
    uint8_t *rxBuffer;               /* RX buffer pointer */
    uint32_t rxBufferSize;           /* RX buffer size */
    uint8_t *txBuffer;               /* TX buffer pointer */
    uint32_t txBufferSize;           /* TX buffer size */
} ETH_Handle_t;

/* Ethernet Frame Structure */
typedef struct {
    uint8_t destination[6];          /* Destination MAC address */
    uint8_t source[6];               /* Source MAC address */
    uint16_t type;                   /* Ethernet type (e.g., 0x0800 for IP) */
    uint8_t *payload;                /* Payload data */
    uint32_t payloadLength;          /* Payload length */
} ETH_Frame_t;

/* Function Prototypes */

/**
 * @brief Initialize Ethernet with given configuration
 * @param handle: Pointer to ETH handle
 * @param config: Pointer to ETH configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_Init(ETH_Handle_t *handle, ETH_Config_t *config);

/**
 * @brief Deinitialize Ethernet
 * @param handle: Pointer to ETH handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_DeInit(ETH_Handle_t *handle);

/**
 * @brief Start Ethernet communication
 * @param handle: Pointer to ETH handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_Start(ETH_Handle_t *handle);

/**
 * @brief Stop Ethernet communication
 * @param handle: Pointer to ETH handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_Stop(ETH_Handle_t *handle);

/**
 * @brief Transmit Ethernet frame
 * @param handle: Pointer to ETH handle
 * @param frame: Pointer to ETH frame structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_TransmitFrame(ETH_Handle_t *handle, ETH_Frame_t *frame);

/**
 * @brief Receive Ethernet frame
 * @param handle: Pointer to ETH handle
 * @param frame: Pointer to ETH frame structure to fill
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_ReceiveFrame(ETH_Handle_t *handle, ETH_Frame_t *frame);

/**
 * @brief Check if Ethernet link is up
 * @param handle: Pointer to ETH handle
 * @return bool: True if link is up, false otherwise
 */
bool ETH_IsLinkUp(ETH_Handle_t *handle);

/**
 * @brief Get Ethernet link speed
 * @param handle: Pointer to ETH handle
 * @return uint32_t: Link speed (ETH_SPEED_10M or ETH_SPEED_100M)
 */
uint32_t ETH_GetLinkSpeed(ETH_Handle_t *handle);

/**
 * @brief Get Ethernet link duplex mode
 * @param handle: Pointer to ETH handle
 * @return uint32_t: Duplex mode (ETH_MODE_FULLDUPLEX or ETH_MODE_HALFDUPLEX)
 */
uint32_t ETH_GetLinkDuplex(ETH_Handle_t *handle);

/**
 * @brief Set MAC address
 * @param handle: Pointer to ETH handle
 * @param macAddr: New MAC address
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_SetMACAddress(ETH_Handle_t *handle, uint8_t *macAddr);

/**
 * @brief Get MAC address
 * @param handle: Pointer to ETH handle
 * @param macAddr: Buffer to store MAC address
 */
void ETH_GetMACAddress(ETH_Handle_t *handle, uint8_t *macAddr);

/**
 * @brief Enable Ethernet interrupts
 * @param handle: Pointer to ETH handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_EnableInterrupts(ETH_Handle_t *handle);

/**
 * @brief Disable Ethernet interrupts
 * @param handle: Pointer to ETH handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef ETH_DisableInterrupts(ETH_Handle_t *handle);

/**
 * @brief Ethernet IRQ Handler - to be called from ETH_IRQn handler
 * @param handle: Pointer to ETH handle
 */
void ETH_IRQHandler(ETH_Handle_t *handle);

/**
 * @brief Ethernet Transmit Complete Callback (to be implemented by user)
 * @param heth: Pointer to HAL ETH handle
 */
void ETH_TxCpltCallback(ETH_HandleTypeDef *heth);

/**
 * @brief Ethernet Receive Complete Callback (to be implemented by user)
 * @param heth: Pointer to HAL ETH handle
 */
void ETH_RxCpltCallback(ETH_HandleTypeDef *heth);

/**
 * @brief Ethernet Error Callback (to be implemented by user)
 * @param heth: Pointer to HAL ETH handle
 */
void ETH_ErrorCallback(ETH_HandleTypeDef *heth);

#endif /* ETH_H */
