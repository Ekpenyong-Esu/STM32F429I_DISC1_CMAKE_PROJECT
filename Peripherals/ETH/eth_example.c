/**
 * @file eth_example.c
 * @brief Ethernet Driver Example Usage
 * @author Generated for STM32F429
 * @date 2025
 *
 * This file demonstrates how to use the Ethernet driver for basic network operations.
 */

#include "eth.h"
#include <string.h>

/* Network constants */
#define MAC_ADDR_LEN            6
#define BROADCAST_BYTE          0xFF
#define ARP_ETHERTYPE           0x0806
#define IPV4_ETHERTYPE          0x0800

/* Example MAC address bytes */
#define MAC_BYTE_1              0x11
#define MAC_BYTE_2              0x22
#define MAC_BYTE_3              0x33
#define MAC_BYTE_4              0x44
#define MAC_BYTE_5              0x55

/* Example IP address bytes */
#define IP_192                  192
#define IP_168                  168
#define IP_SUBNET_1             1
#define IP_HOST_1               1
#define IP_HOST_100             100

/* Example destination MAC */
#define DEST_MAC_1              0xAA
#define DEST_MAC_2              0xBB
#define DEST_MAC_3              0xCC
#define DEST_MAC_4              0xDD
#define DEST_MAC_5              0xEE

/* IPv4 header constants */
#define IPV4_VERSION_IHL        0x45
#define IPV4_TOTAL_LEN_HIGH     0x00
#define IPV4_TOTAL_LEN_LOW      0x3C
#define IPV4_TTL                0x40

/* ICMP data bytes */
#define ICMP_DATA_A             0x61  /* 'a' */
#define ICMP_DATA_B             0x62  /* 'b' */
#define ICMP_DATA_C             0x63  /* 'c' */
#define ICMP_DATA_D             0x64  /* 'd' */
#define ICMP_DATA_E             0x65  /* 'e' */
#define ICMP_DATA_F             0x66  /* 'f' */
#define ICMP_DATA_G             0x67  /* 'g' */
#define ICMP_DATA_H             0x68  /* 'h' */

/* Ethernet handle */
ETH_Handle_t ethHandle;

/* Buffers for Ethernet communication */
#define ETH_MAX_FRAME_SIZE 1518
uint8_t txBuffer[ETH_MAX_FRAME_SIZE];
uint8_t rxBuffer[ETH_MAX_FRAME_SIZE];

/* Example MAC address */
uint8_t macAddress[MAC_ADDR_LEN] = {0x00, MAC_BYTE_1, MAC_BYTE_2, MAC_BYTE_3, MAC_BYTE_4, MAC_BYTE_5};

/* Transfer complete flags */
volatile bool ethTxComplete = false;
static volatile bool ethRxComplete = false;

/**
 * @brief Ethernet Transmit Complete Callback Implementation
 */
void ETH_TxCpltCallback(ETH_HandleTypeDef *heth) {
    UNUSED(heth);
    ethTxComplete = true;
}

/**
 * @brief Ethernet Receive Complete Callback Implementation
 */
void ETH_RxCpltCallback(ETH_HandleTypeDef *heth) {
    UNUSED(heth);
    ethRxComplete = true;
}

/**
 * @brief Ethernet Error Callback Implementation
 */
void ETH_ErrorCallback(ETH_HandleTypeDef *heth) {
    UNUSED(heth);
    /* Handle Ethernet error */
}

/**
 * @brief Initialize Ethernet for basic communication
 */
void ETH_Example_Init(void) {
    /* Configure Ethernet */
    ETH_Config_t ethConfig = {
        .macAddr = {0x00, MAC_BYTE_1, MAC_BYTE_2, MAC_BYTE_3, MAC_BYTE_4, MAC_BYTE_5}, /* MAC address */
        .speed = ETH_SPEED_100M,                           /* 100 Mbps */
        .duplexMode = ETH_FULLDUPLEX_MODE,                 /* Full duplex */
        .checksumMode = ETH_CHECKSUM_BY_SOFTWARE,          /* Software checksum */
        .mediaInterface = ETH_MEDIA_INTERFACE_RMII,        /* RMII interface */
        .vlanTagIdentifier = 0,                            /* No VLAN */
        .vlanTagProtocol = 0                               /* No VLAN */
    };

    /* Assign buffers */
    ethHandle.rxBuffer = rxBuffer;
    ethHandle.rxBufferSize = sizeof(rxBuffer);
    ethHandle.txBuffer = txBuffer;
    ethHandle.txBufferSize = sizeof(txBuffer);

    /* Initialize Ethernet */
    if (ETH_Init(&ethHandle, &ethConfig) != HAL_OK) {
        /* Initialization failed */
        while (1) {
            /* Error loop */
        }
    }

    /* Start Ethernet communication */
    if (ETH_Start(&ethHandle) != HAL_OK) {
        /* Start failed */
        while (1) {
            /* Error loop */
        }
    }

    /* Enable interrupts */
    ETH_EnableInterrupts(&ethHandle);
}

/**
 * @brief Example: Send an ARP request
 */
void ETH_Example_SendARPRequest(void) {
    /* ARP request frame */
    ETH_Frame_t arpFrame;

    /* Broadcast destination */
    memset(arpFrame.destination, BROADCAST_BYTE, MAC_ADDR_LEN);
    /* Source MAC */
    memcpy(arpFrame.source, macAddress, MAC_ADDR_LEN);
    /* ARP type */
    arpFrame.type = ARP_ETHERTYPE; /* ARP */

    /* Simple ARP payload (Who has 192.168.1.1? Tell 192.168.1.100) */
    uint8_t arpPayload[] = {
        0x00, 0x01, /* Hardware type: Ethernet */
        0x08, 0x00, /* Protocol type: IPv4 */
        0x06,       /* Hardware address length */
        0x04,       /* Protocol address length */
        0x00, 0x01, /* Operation: ARP request */
        /* Sender MAC */
        0x00, MAC_BYTE_1, MAC_BYTE_2, MAC_BYTE_3, MAC_BYTE_4, MAC_BYTE_5,
        /* Sender IP: 192.168.1.100 */
        IP_192, IP_168, IP_SUBNET_1, IP_HOST_100,
        /* Target MAC: 00:00:00:00:00:00 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* Target IP: 192.168.1.1 */
        IP_192, IP_168, IP_SUBNET_1, IP_HOST_1
    };

    arpFrame.payload = arpPayload;
    arpFrame.payloadLength = sizeof(arpPayload);

    /* Send ARP request */
    ethTxComplete = false;
    if (ETH_TransmitFrame(&ethHandle, &arpFrame) != HAL_OK) {
        /* Transmission failed */
        return;
    }

    /* Wait for transmission to complete */
    while (!ethTxComplete) {
        /* Wait */
    }
}

/**
 * @brief Example: Send a ping (ICMP echo request)
 */
void ETH_Example_SendPing(void) {
    /* ICMP echo request frame */
    ETH_Frame_t icmpFrame;

    /* Destination MAC (should be resolved via ARP first) */
    uint8_t destMac[MAC_ADDR_LEN] = {DEST_MAC_1, DEST_MAC_2, DEST_MAC_3, DEST_MAC_4, DEST_MAC_5, BROADCAST_BYTE}; /* Example MAC */
    memcpy(icmpFrame.destination, destMac, MAC_ADDR_LEN);
    /* Source MAC */
    memcpy(icmpFrame.source, macAddress, MAC_ADDR_LEN);
    /* IP type */
    icmpFrame.type = IPV4_ETHERTYPE; /* IPv4 */

    /* Simple IPv4 + ICMP payload */
    uint8_t icmpPayload[] = {
        /* IPv4 Header (simplified) */
        IPV4_VERSION_IHL,       /* Version 4, Header length 5 */
        0x00,       /* Type of service */
        IPV4_TOTAL_LEN_HIGH, IPV4_TOTAL_LEN_LOW, /* Total length: 60 bytes */
        0x00, 0x00, /* Identification */
        0x00, 0x00, /* Flags and Fragment offset */
        IPV4_TTL,       /* TTL: 64 */
        0x01,       /* Protocol: ICMP */
        0x00, 0x00, /* Header checksum (calculated) */
        /* Source IP: 192.168.1.100 */
        IP_192, IP_168, IP_SUBNET_1, IP_HOST_100,
        /* Destination IP: 192.168.1.1 */
        IP_192, IP_168, IP_SUBNET_1, IP_HOST_1,

        /* ICMP Header */
        0x08,       /* Type: Echo request */
        0x00,       /* Code: 0 */
        0x00, 0x00, /* Checksum */
        0x00, 0x01, /* Identifier */
        0x00, 0x01, /* Sequence number */
        /* Data */
        ICMP_DATA_A, ICMP_DATA_B, ICMP_DATA_C, ICMP_DATA_D, ICMP_DATA_E, ICMP_DATA_F, ICMP_DATA_G, ICMP_DATA_H /* "abcdefgh" */
    };

    icmpFrame.payload = icmpPayload;
    icmpFrame.payloadLength = sizeof(icmpPayload);

    /* Send ping */
    ethTxComplete = false;
    if (ETH_TransmitFrame(&ethHandle, &icmpFrame) != HAL_OK) {
        /* Transmission failed */
        return;
    }

    /* Wait for transmission to complete */
    while (!ethTxComplete) {
        /* Wait */
    }
}

/**
 * @brief Example: Receive and process Ethernet frames
 */
void ETH_Example_ReceiveFrames(void) {
    ETH_Frame_t receivedFrame;

    /* Check if frame is available */
    ethRxComplete = false;

    if (ETH_ReceiveFrame(&ethHandle, &receivedFrame) == HAL_OK) {
        /* Process received frame based on type */
        switch (receivedFrame.type) {
            case ARP_ETHERTYPE: /* ARP */
                /* Handle ARP packet */
                break;

            case IPV4_ETHERTYPE: /* IPv4 */
                /* Handle IPv4 packet */
                break;

            default:
                /* Unknown packet type */
                break;
        }
    }
}

/**
 * @brief Deinitialize Ethernet
 */
void ETH_Example_DeInit(void) {
    ETH_Stop(&ethHandle);
    ETH_DisableInterrupts(&ethHandle);
    ETH_DeInit(&ethHandle);
}

/* Usage in main.c or application code:

#include "eth_example.h"

int main(void) {
    // System initialization...

    ETH_Example_Init();

    // Example operations
    ETH_Example_SendARPRequest();
    HAL_Delay(100); // Wait for response

    ETH_Example_SendPing();
    HAL_Delay(100); // Wait for response

    // Main loop
    while (1) {
        ETH_Example_ReceiveFrames();
        // Other application code...
    }

    ETH_Example_DeInit();
    return 0;
}

*/
