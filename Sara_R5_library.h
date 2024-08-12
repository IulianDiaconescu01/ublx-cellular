// INCLUDES
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_uart.h"

// General
#define SMALL_RESPONSE_BUFFER_SIZE 64
#define STANDARD_RESPONSE_BUFFER_SIZE 128
#define LARGE_RESPONSE_BUFFER_SIZE 255
#define SIZE_LONG_OP 26
#define SIZE_SHORT_OP 11
#define SIZE_PDP_TYPE 10
#define SIZE_APN 128
#define SIZE_OCT_IP 4
#define MAX_OPS 3             // MAX OPERATORS
#define MAX_APN 3             // MAX APN
#define SARA_R5_NUM_SOCKETS 6 // MAX NUM SOCKETS

// Timing
#define SARA_R5_STANDARD_RESPONSE_TIMEOUT 1000 // 1 SEC TIMEOUT
#define SARA_R5_3_MIN_TIMEOUT 180000           // 3 MIN TIMEOUT
#define SARA_R5_10_SEC_TIMEOUT 10000           // 10 SEC TIMEOUT
#define SARA_R5_IP_CONNECT_TIMEOUT 130000

//Memory
#define MESSAGE_PDP_ACTION_EXTRA_MEMORY 32 //Additional memory allocation  to extra characters
#define OPERATOR_SELECTION_EXTRA_MEMORY 3
#define OPERATOR_SELECTION_MEMORY 10
#define AUTO_OPERATOR_SELECTION_MEMORY 6
#define MESSAGE_PDP_DEF_EXTRA_MEMORY 3
#define RESPONSE_MEMORY 1024
#define RESPONSE_EXTRA_MEMORY 48
#define APN_EXTRA_MEMORY 16
#define CREATE_SOCKET_EXTRA_MEMORY 10
#define CLOSE_SOCKET_EXTRA_MEMORY 10
#define CONNECT_SOCKET_EXTRA_MEMORY 11

// IP
#define SARA_R5_SIZE_IP 16

// Supported AT Commands
// General
#define SARA_R5_COMMAND_AT "AT\r" // AT test
// #define SARA_R5_INFORMATION "AT+CGDCONT=2\r"		// SARA R5 INFORMATION
// #define SARA_R5_INFORMATION2 "AT+USOCL=0\r"		// SARA R5 INFORMATION
#define SARA_RESPONSE_OK "\r\nOK\r\n"             // OK response
#define SARA_RESPONSE_ERROR "\r\nERROR\r\n"       // ERROR response
#define SARA_R5_COMMAND_ECHO_DESACTIVATE "ATE0\r" // Desactivate local echo
// Network service
#define SARA_R5_OPERATOR_SELECTION "AT+COPS" // search operators

// Control and status
#define SARA_R5_COMMAND_FUNCT "AT+CFUN" // Functionallity
// Packet switched data services
#define SARA_R5_MESSAGE_PDP_ACTION "AT+UPSDA"    // Perform the action for the specified PSD profile
#define SARA_R5_NETWORK_ASSIGNED_DATA "AT+UPSND" // Packet switched network-assigned data
#define SARA_R5_MESSAGE_PDP_DEF "AT+CGDCONT"     // Packet switched Data Profile context definition
#define SARA_R5_MESSAGE_PDP_DEF2 "AT+CGDCONT?\r"
// IP
#define SARA_R5_CREATE_SOCKET "AT+USOCR"      // Create a new socket
#define SARA_R5_CREATE_SOCKET2 "AT+USOCL=3\r" // Create a new socket
#define SARA_R5_CLOSE_SOCKET "AT+USOCL"       // Close the socket
#define SARA_R5_CONNECT_SOCKET "AT+USOCO"     // Socket Connect
#define SARA_R5_WRITE_SOCKET "AT+USOWR"       // Write data to a socket
#define SARA_R5_WRITE_UDP_SOCKET "AT+USOST"   // Write data to a UDP socket

typedef enum
{
  SARA_R5_PSD_ACTION_RESET = 0,  // Reset the PSD profile
  SARA_R5_PSD_ACTION_STORE,      // Store the current PSD profile in memory
  SARA_R5_PSD_ACTION_LOAD,       // Load the PSD profile from memory
  SARA_R5_PSD_ACTION_ACTIVATE,   // Activate the PSD profile
  SARA_R5_PSD_ACTION_DESACTIVATE // Deactivate the PSD profile
} SARA_R5_pdp_actions_t;

typedef struct
{
  uint8_t stat;                // Status of the operator. E.g., 0 for unknown, 1 for available, 2 for current, etc.
  char shortOp[SIZE_SHORT_OP]; // Short alphanumeric format of the operator's name (e.g., "Vodafone", "AT&T")
  char longOp[SIZE_LONG_OP];   // Long alphanumeric format of the operator's name
  unsigned long numOp;         // Numeric format of the operator's name (often MCC-MNC format)
  uint8_t act;                 // Access technology used by the operator. E.g., 0 for GSM, 1 for UMTS, etc.
} SARA_R5_operator_stats;

typedef enum
{
  SARA_R5_TCP = 6,
  SARA_R5_UDP = 17
} SARA_R5_socket_protocol_t;

typedef enum
{
  AUTOMATIC = 0,       // Automatic network selection mode
  MANUAL_MODE = 1,     // Manual network selection mode
  DEREGISTER_MODE = 2, // Deregister from network and do not attempt to register again
} SARA_R5_mode_action;

// Represents an IP address with four integer components
typedef struct
{
  int first_ip;  // First octet of the IP address
  int second_ip; // Second octet of the IP address
  int third_ip;  // Third octet of the IP address
  int fourth_ip; // Fourth octet of the IP address
} Ip_adress;

// Represents an Access Point Name (APN) configuration
typedef struct
{
  char apn[SIZE_APN]; // APN string
} myApn;

// Defines the types of Packet Data Protocol (PDP) contexts
typedef enum
{
  PDP_TYPE_INVALID = -1, // Represents an invalid or undefined PDP type
  PDP_TYPE_IP = 0,       // Internet Protocol (IP) based PDP context
  PDP_TYPE_NONIP = 1,    // Non-IP based PDP context (e.g., for non-IP data delivery)
  PDP_TYPE_IPV4V6 = 2,   // Dual-stack IP (IPv4 and IPv6) PDP context
  PDP_TYPE_IPV6 = 3      // IPv6 based PDP context
} SARA_R5_pdp_type;

typedef enum
{
  SARA_R5_ERROR_INVALID = -1,        // Invalid value or unknown error
  SARA_R5_ERROR_SUCCESS = 0,         // Successful operation
  SARA_R5_ERROR_OUT_OF_MEMORY,       // Out of memory error
  SARA_R5_ERROR_TIMEOUT,             // Timeout error
  SARA_R5_ERROR_UNEXPECTED_PARAM,    // Unexpected or incorrect parameter
  SARA_R5_ERROR_UNEXPECTED_RESPONSE, // Unexpected response received
  SARA_R5_ERROR_NO_RESPONSE,         // No response received
  SARA_R5_ERROR_DEREGISTERED,        // Device has deregistered
  SARA_R5_ERROR_ZERO_READ_LENGTH,    // Zero read length in read operation
  SARA_R5_ERROR_ERROR,               // Generic error
  SARA_R5_ERROR_INVALID_SOCKET       // Invalid Socket
} SARA_R5_error_t;

// FUNCTION TO ALLOCATE MEMORY
char *saraR5CallocChar(size_t num);

// FUNCTION TO INITIALIZE THE MODULE WITHOUT ECHO.
bool saraR5Init(const char *expectedResponse, const char *buffer);

// FUNCTIONS TO SEND & RECEIVE COMMANDS
bool saraR5SendDataUART(const uint8_t *data, uint32_t size);
bool saraR5ReceiveDataUART(const uint8_t *buffer, uint8_t size, unsigned long timeout);
bool saraR5ReceiveCommand(const char *buffer, uint8_t size, unsigned long timeout);
bool saraR5SendCommand(const uint8_t *command);
bool saraR5SendCommandWithResponse(const char *command, const char *expectedResponse, const char *buffer, uint8_t size, unsigned long timeout);

// PACKET SWITCHED DATA
uint8_t saraR5PerformPDPaction(int profile, SARA_R5_pdp_actions_t action, const char *buffer, uint8_t size);

// NETWORK SERVICE
uint8_t saraR5GetOperators(SARA_R5_operator_stats *opRet, int maxOps, const char *buffer, uint8_t size);
uint8_t saraR5NetworkMode(SARA_R5_mode_action mode, const char *buffer, uint8_t size);
uint8_t saraR5AutomaticOperatorSelection(const char *buffer, uint8_t size);

// FUNCTIONS FOR APN
uint8_t saraR5GetAPN(int cid, myApn *apn, Ip_adress *ip, SARA_R5_pdp_type *pdpType);
uint8_t saraR5SetAPN(uint8_t cid, SARA_R5_pdp_type pdpType, char *apn, const char *buffer, uint8_t size);

// FUNCTIONS FOR SOCKETS
int saraR5SocketOpen(SARA_R5_socket_protocol_t protocol, unsigned long localPort);
uint8_t saraR5socketClose(int socket, unsigned long timeout, const char *buffer, uint8_t size);
uint8_t saraR5SocketConnect(int socket, Ip_adress ip, unsigned int port, const char *buffer, uint8_t size);
uint8_t saraR5SocketConnect2(int socket, const char *address, unsigned int port, const char *buffer, uint8_t size);
uint8_t saraR5SocketWriteUDP(int socket, const char *address, int port, const char *str, int len);