#include "SARA_R5_library.h"

extern UART_HandleTypeDef huart1;

/**
 * Allocates memory for an array of 'num' characters and initializes it to zero.
 * @param num The number of characters to allocate.
 * @return A pointer to the allocated memory, or NULL if the allocation fails.
 */
char *saraR5CallocChar(size_t num)
{
	// Allocate memory for an array of 'num' characters and initialize to zero
	char *memory = calloc(num, sizeof(char));
	if (memory == NULL)
	{
		// Handle the error, e.g., return NULL or print an error message
		return NULL;
	}
	return memory;
}

/**
 * Initializes the SARA R5 module by sending specific commands and checking responses.
 * @param expectedResponse The response string expected from the module.
 * @param buffer A buffer to store the received data.
 * @return true if initialization is successful, false otherwise.
 */
bool saraR5Init(const char *expectedResponse, const char *buffer)
{

	// Desactivate echo
	if (!saraR5SendCommand((const uint8_t *)SARA_R5_COMMAND_ECHO_DESACTIVATE))
	{
		return false; // Failed to send ECHO DESACTIVATE command
	}

	// Send AT command
	if (!saraR5SendCommand((const uint8_t *)SARA_R5_COMMAND_AT))
	{
		return false; // Failed to send AT command
	}

	// Receive OK to continue
	saraR5ReceiveCommand(buffer, STANDARD_RESPONSE_BUFFER_SIZE, SARA_R5_10_SEC_TIMEOUT);

	// Check if the expected response is in the buffer
	if (strcmp(buffer, expectedResponse) == 0)
	{
		return true; // Expected response received
	}
	else
	{
		return false; // Expected response not received
	}
}

/**
 * Sends data via UART.
 * @param data Pointer to the data to be sent.
 * @param size The number of bytes to send.
 * @return true if transmission is successful, false otherwise.
 */
bool saraR5SendDataUART(const uint8_t *data, uint32_t size)
{
	// Send data via UART
	if (HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY) == HAL_OK)
	{
		return true; // Successful transmission
	}
	else
	{
		return false; // Transmission error
	}
}

/**
 * Receives data via UART.
 * @param buffer Pointer to the buffer where received data will be stored.
 * @param size The number of bytes to receive.
 * @param timeout The timeout in milliseconds.
 * @return true if reception is successful, false otherwise.
 */
bool saraR5ReceiveDataUART(const uint8_t *buffer, uint8_t size, unsigned long timeout)
{
	// Receive data via UART
	if (HAL_UART_Receive(&huart1, (uint8_t *)buffer, size, timeout) == HAL_OK)
	{
		return true; // Successful reception
	}
	else
	{
		return false; // Error in reception
	}
}

/**
 * Receives a command using the saraR5ReceiveDataUART function.
 * @param buffer Pointer to the buffer where received data will be stored.
 * @param size The number of bytes to receive.
 * @param timeout The timeout in milliseconds.
 * @return true if reception is successful, false otherwise.
 */
bool saraR5ReceiveCommand(const char *buffer, uint8_t size, unsigned long timeout)
{
	// Receive command using saraR5ReceiveDataUART function
	return saraR5ReceiveDataUART((uint8_t *)buffer, size, timeout);
}

/**
 * Sends a command using the saraR5SendDataUART function.
 * @param command Pointer to a uint8_t array containing the command to be sent.
 * The command must be properly formatted and terminated with a null character.
 * @return true if the command is successfully sent, false otherwise.
 */
bool saraR5SendCommand(const uint8_t *command)
{
	// Send command using saraR5SendDataUART function
	return saraR5SendDataUART(command, strlen((const char *)command));
}

/**
 * Sends a command and waits for a specific response using the saraR5SendCommand and saraR5ReceiveCommand functions.
 * @param command Pointer to a string containing the command to be sent.
 * @param expectedResponse Pointer to a string containing the expected response to check for in the received data.
 * @param buffer Pointer to a buffer where the received data will be stored.
 * @param size The number of bytes to be read into the buffer.
 * @param timeout The timeout in milliseconds for receiving the response.
 * @return true if the function gets the right response in time, false if it does not.
 */
bool saraR5SendCommandWithResponse(const char *command, const char *expectedResponse, const char *buffer, uint8_t size, unsigned long timeout)
{
	// Use saraR5SendCommand function to send command
	saraR5SendCommand((const uint8_t *)command);
	// Use saraR5ReceiveCommand function to receive command
	saraR5ReceiveCommand(buffer, size, timeout);
	// See if the expected response "\r\nOK\r\n" is in the buffer
	if (strstr(buffer, expectedResponse) != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Performs an action on a PDP (Packet Data Protocol) profile.
 * @param profile The PDP profile number.
 * @param action The action to perform on the PDP profile.
 * @param buffer A place to store data received from the action.
 * @param size How big the buffer is in bytes.
 * @return A number indicating if the action was successful, ran out of memory,
 *         encountered an error, or got no response.
 */
uint8_t saraR5PerformPDPaction(int profile, SARA_R5_pdp_actions_t action, const char *buffer, uint8_t size)
{

	char *command;

	// Calls the calloc function
	/**
	 * Allocates extra memory for the command string, adding 32 bytes to the length of SARA_R5_MESSAGE_PDP_ACTION.
	 * This buffer space accommodates additional command parameters and ensures safety against buffer overflow.
	 */
	command = saraR5CallocChar(strlen(SARA_R5_MESSAGE_PDP_ACTION) + MESSAGE_PDP_ACTION_EXTRA_MEMORY);

	if (command == NULL)
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	// Save the command as e.g "AT+UPSDA = 2,2"
	sprintf(command, "%s=%d,%d\r", SARA_R5_MESSAGE_PDP_ACTION, profile, action);

	// Send command and check response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer, size, SARA_R5_10_SEC_TIMEOUT))
	{
		// If we receive Error or Nothing we free memory and return Error
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			free(command);
			return SARA_R5_ERROR_ERROR;
		}
		else
		{
			free(command);
			return SARA_R5_ERROR_NO_RESPONSE;
		}
	}
	free(command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Gets a list of mobile network operators available.
 * @param opRet Where to store the list of operators.
 * @param maxOps The maximum number of operators to find.
 * @param buffer A place to store data received from the action.
 * @param size How big the buffer is in bytes.
 * @return The number of operators found, or an error code if something goes wrong.
 */
uint8_t saraR5GetOperators(SARA_R5_operator_stats *opRet, int maxOps, const char *buffer, uint8_t size)
{

	uint8_t opsSeen = 0;
	char *response;
	char *command;
	int op = 0;
	char *opBegin;
	char *opEnd;
	int stat;
	char longOp[SIZE_LONG_OP];
	char shortOp[SIZE_SHORT_OP];
	int act;
	unsigned long numOp;

	// Allocate memory for command
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_OPERATOR_SELECTION) + OPERATOR_SELECTION_EXTRA_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Send the AT+COPS = ?, to see the available networks
	sprintf(command, "%s=?\r", SARA_R5_OPERATOR_SELECTION);

	// Allocate memory for response
	int responseSize = (maxOps + 1) * RESPONSE_EXTRA_MEMORY;
	response = saraR5CallocChar(responseSize);
	if (response == NULL)
	{
		free(command);
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Send AT+COPS = 0,0 to set to automatic
	saraR5AutomaticOperatorSelection(buffer, size);
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, response, size, SARA_R5_3_MIN_TIMEOUT))
	{
		free(command);
		free(response);
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			return SARA_R5_ERROR_ERROR;
		}
		return SARA_R5_ERROR_NO_RESPONSE;
	}
	// Parse the response to extract operator information
	opBegin = response;

	// Sample responses format: +COPS: (1,"313 100","313 100","313100",8),(2,"AT&T","AT&T","310410",8), ...
	for (; op < maxOps; op++)
	{
		opBegin = strchr(opBegin, '(');

		opEnd = strchr(opBegin, ')');
		memset(longOp, 0, sizeof(longOp));
		memset(shortOp, 0, sizeof(shortOp));
		int sscanRead = sscanf(opBegin, "(%d,\"%[^\"]\",\"%[^\"]\",\"%lu\",%d)%*s", &stat, longOp, shortOp, &numOp, &act);
		// Search for other possible patterns here
		if (sscanRead == 5)
		{
			opRet[op].stat = stat;
			strcpy(opRet[op].longOp, longOp);
			strcpy(opRet[op].shortOp, shortOp);
			opRet[op].numOp = numOp;
			opRet[op].act = act;
			opsSeen += 1;
		}
		opBegin = opEnd + 1;
		// Move opBegin to beginning of next value
	}

	free(command);
	free(response);
	return opsSeen;
}

/**
 * Gets the Access Point Name (APN) and IP address information for a mobile network.
 * @param cid The context ID for which the APN information is requested.
 * @param apn Where to store the APN information.
 * @param ip Where to store the IP address information.
 * @param pdpType Where to store the type of Packet Data Protocol (PDP).
 * @return A number indicating if the function was successful, or an error code if something went wrong.
 */
uint8_t saraR5GetAPN(int cid, myApn *apn, Ip_adress *ip, SARA_R5_pdp_type *pdpType)
{

	char *command;
	char *response;
	int op = 0;
	bool success = false;

	// Allocate memory for the command
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_MESSAGE_PDP_DEF) + MESSAGE_PDP_DEF_EXTRA_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Ask for active PDP context
	sprintf(command, "%s?\r", SARA_R5_MESSAGE_PDP_DEF);

	// Allocate memory for the response
	response = saraR5CallocChar(RESPONSE_MEMORY);
	if (response == NULL)
	{
		free(command);
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Send command and check response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, response, LARGE_RESPONSE_BUFFER_SIZE, SARA_R5_STANDARD_RESPONSE_TIMEOUT))
	{
		if (strstr(response, SARA_RESPONSE_ERROR))
		{
			free(command);
			return SARA_R5_ERROR_ERROR;
		}
	}

	// Parse the response to extract APN information
	int rcid = -1;
	char *searchPtr = response;
	// Sample responses:
	//  +CGDCONT: 1,"IP","payandgo.o2.co.uk.mnc010.mcc234.gprs","10.160.182.234",0,0,0,2,0,0,0,0,0,0
	for (; op < MAX_OPS; op++)
	{
		int scanned = 0;
		// Find the first/next occurrence of +CGDCONT:
		searchPtr = strstr(searchPtr, "+CGDCONT:");
		if (searchPtr != NULL)
		{

			char strPdpType[SIZE_PDP_TYPE] = "";
			char strApn[SIZE_APN];
			int ipOct[SIZE_OCT_IP] = {0};

			searchPtr += strlen("+CGDCONT:");
			while (*searchPtr == ' ')
				searchPtr++; // Skip spaces

			scanned = sscanf(searchPtr, "%d,\"%[^\"]\",\"%[^\"]\",\"%d.%d.%d.%d", &rcid, strPdpType, strApn, &ipOct[0], &ipOct[1], &ipOct[2], &ipOct[3]);
			if (scanned == 7)
			{
				success = true;
				// If scanned == 7 we can save the result
				strcpy(apn[op].apn, strApn);
				ip[op].first_ip = ipOct[0];
				ip[op].second_ip = ipOct[1];
				ip[op].third_ip = ipOct[2];
				ip[op].fourth_ip = ipOct[3];

				if (pdpType)
				{
					*pdpType = (0 == strcmp(strPdpType, "IPV4V6")) ? PDP_TYPE_IPV4V6 : (0 == strcmp(strPdpType, "IPV6")) ? PDP_TYPE_IPV6
																				   : (0 == strcmp(strPdpType, "IP"))	 ? PDP_TYPE_IP
																														 : PDP_TYPE_INVALID;
				}
			}
		}
		else // We don't have a match so let's clear the APN and IP address
		{
			if (apn)
				apn->apn[0] = '\0';
			if (pdpType)
				*pdpType = PDP_TYPE_INVALID;
			if (ip)
				memset(ip, 0, sizeof(*ip));
		}
	}
	free(command);
	free(response);
	return success ? SARA_R5_ERROR_SUCCESS : SARA_R5_ERROR_NO_RESPONSE; // Returns SUCCESS if at least one context was processed, otherwise returns NO_RESPONSE
}

uint8_t saraR5SetAPN(uint8_t cid, SARA_R5_pdp_type pdpType, char *apn, const char *buffer, uint8_t size)
{

	char *command;
	char pdpStr[8] = "";

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_MESSAGE_PDP_DEF) + strlen(apn + APN_EXTRA_MEMORY));
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Convert PDP type to string representation
	switch (pdpType)
	{
	case PDP_TYPE_INVALID:
		free(command);
		return SARA_R5_ERROR_UNEXPECTED_PARAM;
	case PDP_TYPE_IP:
		memcpy(pdpStr, "IP", 2);
		break;
	case PDP_TYPE_NONIP:
		memcpy(pdpStr, "NONIP", 5);
		break;
	case PDP_TYPE_IPV4V6:
		memcpy(pdpStr, "IPV4V6", 6);
		break;
	case PDP_TYPE_IPV6:
		memcpy(pdpStr, "IPV6", 4);
		break;
	}

	// Send the command AT+CGDCONT = CID, TYPE, APN
	sprintf(command, "%s=%d,\"%s\",\"%s\"\r", SARA_R5_MESSAGE_PDP_DEF, cid, pdpStr, apn);

	if (!saraR5SendCommandWithResponse((const char *)command, (const char *)SARA_RESPONSE_OK, buffer, size, SARA_R5_10_SEC_TIMEOUT))
	{
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			free(command);
			return SARA_R5_ERROR_ERROR;
		}
		else
		{
			free(command);
			return SARA_R5_ERROR_NO_RESPONSE;
		}
	}
	free(command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Sets the Access Point Name (APN) for a mobile network connection.
 * @param cid The context ID where the APN is to be set.
 * @param pdpType The type of Packet Data Protocol (PDP) to use.
 * @param apn The APN name to be set for the connection.
 * @param buffer A place to store data received from the action.
 * @param size How big the buffer is in bytes.
 * @return A number indicating if the APN was successfully set, or an error code if something went wrong.
 */
uint8_t saraR5NetworkMode(SARA_R5_mode_action mode, const char *buffer, uint8_t size)
{

	// Function to select the mode of network
	char *command;

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_OPERATOR_SELECTION) + OPERATOR_SELECTION_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Construct the network mode command
	sprintf(command, "%s=%d\r", SARA_R5_OPERATOR_SELECTION, mode);

	// Send the command and check for the response
	if (!saraR5SendCommandWithResponse((const char *)command, (const char *)SARA_RESPONSE_OK, buffer, size, SARA_R5_10_SEC_TIMEOUT))
	{
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			free(command);
			return SARA_R5_ERROR_ERROR;
		}
	}

	free(command); // Free memory after use
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Sets the device to choose a mobile network operator automatically.
 * @param buffer A place to store data received after sending the command.
 * @param size The size of the buffer in bytes.
 * @return A number indicating if the operation was successful, or an error code if something went wrong.
 */
uint8_t saraR5AutomaticOperatorSelection(const char *buffer, uint8_t size)
{

	// Function to set the device to automatic operator selection mode
	char *command;

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_OPERATOR_SELECTION) + AUTO_OPERATOR_SELECTION_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Construct the command for automatic operator selection
	sprintf(command, "%s=0,0\r", SARA_R5_OPERATOR_SELECTION);

	// Send the command and check for the response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer, size, SARA_R5_3_MIN_TIMEOUT))
	{
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			free(command);
			return SARA_R5_ERROR_ERROR;
		}
	}

	free(command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Creates a network socket using a specified protocol and local port.
 * @param protocol The type of protocol (TCP, UDP, etc.) for the socket connection.
 * @param localPort The local port number to be used for the socket. If set to 0, the port is assigned automatically.
 * @return Returns the socket ID if the socket is successfully opened, or an error code in case of a failure.
 */
int saraR5SocketOpen(SARA_R5_socket_protocol_t protocol, unsigned long localPort)
{

	char *command;
	char *response;
	int sockId = -1;
	char *responseStart;

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_CREATE_SOCKET) + CREATE_SOCKET_EXTRA_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Construct the socket open command
	if (localPort == 0)
		sprintf(command, "%s=%d\r", SARA_R5_CREATE_SOCKET, (int)protocol);
	else
		sprintf(command, "%s=%d,%lu\r", SARA_R5_CREATE_SOCKET, (int)protocol, localPort);

	// Allocate memory for the response
	response = saraR5CallocChar(STANDARD_RESPONSE_BUFFER_SIZE);
	if (response == NULL)
	{
		free(command);
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}
	// Send the command and check for the response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, response, STANDARD_RESPONSE_BUFFER_SIZE, SARA_R5_STANDARD_RESPONSE_TIMEOUT))
	{
		free(command);
		free(response);
		return SARA_R5_ERROR_ERROR;
	}

	// Parse the response to extract socket ID
	responseStart = strstr(response, "+USOCR:");
	responseStart += strlen("+USOCR:"); //  Move searchPtr to first char
	while (*responseStart == ' ')
		responseStart++;				  // skip spaces
	sscanf(responseStart, "%d", &sockId); // It extracts an integer from responseStart and stores it in sockId.

	free(command);
	free(response);

	// Return the socket ID or an error code
	return (sockId != -1) ? sockId : SARA_R5_ERROR_INVALID_SOCKET;
}

/**
 * Closes a specified network socket.
 * @param socket The ID of the socket to be closed.
 * @param timeout The time in milliseconds to wait for the socket to close.
 *                Use the standard timeout for immediate closure.
 * @param buffer A memory area to store the response from the close command.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the socket is closed properly, or an error code if the closure fails.
 */
uint8_t saraR5socketClose(int socket, unsigned long timeout, const char *buffer, uint8_t size)
{

	char *command;
	char *response;

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_CLOSE_SOCKET) + CLOSE_SOCKET_EXTRA_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Allocate memory for the response
	response = saraR5CallocChar(STANDARD_RESPONSE_BUFFER_SIZE);
	if (response == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}
	// If the standard response is 'timeout', then use the first option; otherwise, use the second option.
	const char *format = (SARA_R5_STANDARD_RESPONSE_TIMEOUT == timeout) ? "%s=%d,1\r" : "%s=%d\r";

	// Construct the socket close command
	sprintf(command, format, SARA_R5_CLOSE_SOCKET, socket);

	// Send the command and check for the response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer, size, SARA_R5_10_SEC_TIMEOUT))
	{
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			free(command);
			free(response);
			return SARA_R5_ERROR_ERROR;
		}
	}
	free(command);
	free(response);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Connects an existing network socket to a specific IP address and port.
 * @param socket The ID of the socket to be connected.
 * @param ip The IP address to connect the socket to, provided as a structured format.
 * @param port The port number to connect the socket to.
 * @param buffer A memory area to store the response from the connect command.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the connection is established, or an error code if the connection fails.
 */
uint8_t saraR5SocketConnect(int socket, Ip_adress ip, unsigned int port, const char *buffer, uint8_t size)
{

	char *charAddress = saraR5CallocChar(SARA_R5_SIZE_IP);

	if (charAddress == NULL)
	{
		return false;
	}

	memset(charAddress, 0, SARA_R5_SIZE_IP);

	// Convert the IP address structure to a string representation
	sprintf(charAddress, "%d.%d.%d.%d", ip.first_ip, ip.second_ip, ip.third_ip, ip.fourth_ip);

	// Call the function to send the command to connect
	uint8_t connectResult = saraR5SocketConnect2(socket, (const char *)charAddress, port, buffer, size);

	free(charAddress);
	return connectResult;
}

/**
 * Establishes a network connection for a given socket to a specified IP address and port.
 * @param socket The ID of the socket to be connected.
 * @param address The IP address in string format to connect the socket to.
 * @param port The port number to connect the socket to.
 * @param buffer A memory area to store the response from the connection attempt.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the connection is successfully made, or an error code if the connection attempt fails.
 */
uint8_t saraR5SocketConnect2(int socket, const char *address, unsigned int port, const char *buffer, uint8_t size)
{

	char *command;

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_CONNECT_SOCKET) + strlen(address) + CONNECT_SOCKET_EXTRA_MEMORY);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Construct the command to connect the socket
	sprintf(command, "%s=%d,\"%s\",%d\r", SARA_R5_CONNECT_SOCKET, socket, address, port);

	// Send the command and check for the response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer, size, SARA_R5_IP_CONNECT_TIMEOUT))
	{
		if (strstr(buffer, SARA_RESPONSE_ERROR))
		{
			free(command);
			return SARA_R5_ERROR_ERROR;
		}
	}

	free(command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Sends data through a UDP socket to a specified IP address and port.
 * @param socket The ID of the UDP socket to use for sending data.
 * @param address The destination IP address in string format.
 * @param port The destination port number.
 * @param str The string data to be sent.
 * @param len The length of the data to send. If set to -1, the function calculates the length automatically.
 * @return Returns a success code if the data is sent successfully, or an error code if the sending fails.
 */
uint8_t saraR5SocketWriteUDP(int socket, const char *address, int port, const char *str, int len)
{

	char *command;
	char *response;
	char *responseData;

	// Determine the data length
	int dataLen = len == -1 ? strlen(str) : len;

	// Allocate memory for the command string
	// This extra buffer space accommodates additional command parameters and ensures security against buffer overflow.
	command = saraR5CallocChar(SMALL_RESPONSE_BUFFER_SIZE);
	if (command == NULL)
	{
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Allocate memory for the command response
	response = saraR5CallocChar(STANDARD_RESPONSE_BUFFER_SIZE);
	if (response == NULL)
	{
		free(command);
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Allocate memory for the data response
	responseData = saraR5CallocChar(len);
	if (response == NULL)
	{
		free(command);
		free(response);
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Construct the command to write data to the UDP socket
	sprintf(command, "%s=%d,\"%s\",%d,%d\r", SARA_R5_WRITE_UDP_SOCKET, socket, address, port, dataLen);

	// Send the command and check for the response
	if (!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, response, STANDARD_RESPONSE_BUFFER_SIZE, SARA_R5_STANDARD_RESPONSE_TIMEOUT * 5))
	{
		free(command);
		free(response);
		free(responseData);
		return SARA_R5_ERROR_ERROR;
	}

	if (!saraR5SendCommandWithResponse(str, SARA_RESPONSE_OK, responseData, len, SARA_R5_STANDARD_RESPONSE_TIMEOUT))
	{
		free(command);
		free(response);
		free(responseData);
		return SARA_R5_ERROR_ERROR;
	}

	free(command);
	free(response);
	free(responseData);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Sets the MQTT client ID for a MQTT profile.
 * @param clientId The MQTT client ID to be set.
 * @param buffer A memory area to store the response from the setting attempt.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the client ID is successfully set, or an error code if the attempt fails.
 */
uint8_t saraR5SetMQTTclientId(const char *clientId, const char *buffer, int size)
{
	char *command;
	
	// Allocate memory for the command string
	// This extra memory ensures there is sufficient space for the MQTT profile and client ID, preventing buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_MQTT_PROFILE) + strlen(clientId) + SARA_R5_MQTT_CLIENT_EXTRA_MEMMORY);
	if (command == NULL)
	{
		// Return an error code if memory allocation fails
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}
	
	// Construct the command to set the MQTT client ID
	// The command is formatted with the MQTT profile and client ID.
	sprintf(command, "%s=%d,\"%s\"\r", SARA_R5_MQTT_PROFILE, SARA_R5_MQTT_PROFILE_CLIENT_ID, clientId);
	
	// Send the command and check for the response
	// If the response is not OK, free the memory and return an error code.
	if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer, size,SARA_R5_STANDARD_RESPONSE_TIMEOUT))
	{
		// Free the allocated memory and return an error code if the response is not as expected
		free (command);
		return SARA_R5_ERROR_ERROR;
	}
	// Free the allocated memory for the command string and return a success code
	free (command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Sets the MQTT server details for a MQTT profile.
 * @param serverName The name of the server to be set in the MQTT profile.
 * @param port The port number for the MQTT server.
 * @param buffer A memory area to store the response from the setting attempt.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the server details are successfully set, or an error code if the attempt fails.
 */
uint8_t saraR5SetMQTTserver(const char *serverName, int port, const char *buffer, int size)
{
	char *command;
	
	// Allocate memory for the command string
	// This extra memory is to ensure there's enough space for the MQTT profile, server name, and port to prevent buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_MQTT_PROFILE) + strlen(serverName) + SARA_R5_MQTT_SERVER_EXTRA_MEMMORY);
	if (command == NULL)
	{
		// Return an error code if memory allocation fails
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}
	
	// Construct the command to set the MQTT server details
	// The command is formatted with the MQTT profile, server name, and port.
	sprintf(command, "%s=%d,\"%s\",%d\r", SARA_R5_MQTT_PROFILE, SARA_R5_MQTT_PROFILE_SERVERNAME, serverName, port);
	
	// Send the command and check for the response
	// If the response is not OK, free the memory and return an error code.
	if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer,size,SARA_R5_STANDARD_RESPONSE_TIMEOUT))
	{
		// Free the allocated memory and return an error code if the response is not as expected
		free (command);
		return SARA_R5_ERROR_ERROR;
	}
	// Free the allocated memory for the command string and return a success code
	free (command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Initiates a connection to an MQTT server using predefined MQTT profile settings.
 * @param buffer A memory area to store the response from the connection attempt.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the MQTT connection is successfully initiated, or an error code if the attempt fails.
 */
uint8_t saraR5MQTTconect(const char *buffer, int size)
{
	char *command;
	
	// Allocate memory for the command string
	// This extra memory ensures there's enough space for the MQTT command plus additional space to prevent buffer overflow.
	command = saraR5CallocChar(strlen(SARA_R5_MQTT_COMMAND) + SARA_R5_MQTT_CONNECTION_EXTRA_MEMMORY);
	if (command == NULL)
	{
		// Return an error code if memory allocation fails
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}
	
	// Construct the command to initiate the MQTT connection
	// The command is formatted with the MQTT connection command.
	sprintf(command, "%s=%d\r", SARA_R5_MQTT_COMMAND, SARA_R5_MQTT_COMMAND_LOGIN);
	
	// Send the command and check for the response
	// If the response is not OK, free the memory and return an error code.
	if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer,size,SARA_R5_STANDARD_RESPONSE_TIMEOUT))
	{
		// Free the allocated memory and return an error code if the response is not as expected
		free (command);
		return SARA_R5_ERROR_ERROR;
	}
	
	// Free the allocated memory for the command string and return a success code
	free (command);
	return SARA_R5_ERROR_SUCCESS;
}

/**
 * Disconnects from the MQTT server.
 * @param buffer A memory area to store the response from the disconnection attempt.
 * @param size The size of the buffer in bytes.
 * @return Returns a success code if the MQTT disconnection is successfully completed, or an error code if the attempt fails.
 */
uint8_t saraR5MQTTdisconnect(const char* buffer, int size)
{
    char *command;
    
    // Allocate memory for the command string
    // This extra memory is to ensure there's enough space for the MQTT command and to prevent buffer overflow.
    command = saraR5CallocChar(strlen(SARA_R5_MQTT_COMMAND) + SARA_R5_MQTT_CONNECTION_EXTRA_MEMMORY);
    if (command == NULL)
    {
    	// Return an error code if memory allocation fails
    	return SARA_R5_ERROR_OUT_OF_MEMORY;
    }
    
    // Construct the command to disconnect from the MQTT server
    // The command is formatted with the MQTT disconnection command.
    sprintf(command, "%s=%d\r", SARA_R5_MQTT_COMMAND, SARA_R5_MQTT_COMMAND_LOGOUT);

    // Send the command and check for the response
    // If the response is not OK, free the memory and return an error code.	
    if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer,size,SARA_R5_STANDARD_RESPONSE_TIMEOUT)) // NULL i 0 per no posar un buffer
    {
    	// Free the allocated memory and return an error code if the response is not as expected
    	free (command);
    	return SARA_R5_ERROR_ERROR;
    }
    
    // Free the allocated memory for the command string and return a success code
    free (command);
    return SARA_R5_ERROR_SUCCESS;
}

/**
 * Subscribes to a specified MQTT topic with a maximum Quality of Service (QoS) level.
 * @param max_Qos The maximum Quality of Service level for the subscription.
 * @param topic The MQTT topic to subscribe to.
 * @return Returns a success code if the subscription is successfully created, or an error code if the attempt fails.
 */
uint8_t saraR5SubscribeMQTTtopic(int max_Qos, const char *topic)
{
  char *command;
  
  // Allocate memory for the command string
  // This extra memory accounts for the MQTT command, topic, and additional space to prevent buffer overflow.
  command = saraR5CallocChar(strlen(SARA_R5_MQTT_COMMAND) + strlen(topic) + SARA_R5_MQTT_TOPIC_EXTRA_MEMMORY + strlen(topic));
  if (command == NULL)
  {
	  // Return an error code if memory allocation fails
	  return SARA_R5_ERROR_OUT_OF_MEMORY;
  }
  
  // Construct the command to subscribe to the MQTT topic
  // The command is formatted with the MQTT subscribe command, maximum QoS, and topic.
  sprintf(command, "%s=%d,%d,\"%s\"\r", SARA_R5_MQTT_COMMAND, SARA_R5_MQTT_COMMAND_SUBSCRIBE, max_Qos, topic);
  
  // Send the command and check for the response
  // If the response is not OK, free the memory and return an error code.
  // Note: NULL and 0 are passed to avoid using a buffer for the response.
  if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, NULL,0,SARA_R5_STANDARD_RESPONSE_TIMEOUT)) // NULL i 0 per no posar un buffer
  {
	  // Free the allocated memory and return an error code if the response is not as expected
	  free (command);
	  return SARA_R5_ERROR_ERROR;
  }

  // Free the allocated memory for the command string and return a success code
  free (command);
  return SARA_R5_ERROR_SUCCESS;
}

/**
 * Unsubscribes from a specified MQTT topic.
 * @param topic The MQTT topic to unsubscribe from.
 * @return Returns a success code if the unsubscription is successfully processed, or an error code if the attempt fails.
 */
uint8_t saraR5UnsubscribeMQTTtopic(const char *topic)
{
  char *command;

  // Allocate memory for the command string
  // This extra memory is to ensure there's enough space for the MQTT command and topic, and to prevent buffer overflow.
  command = saraR5CallocChar(strlen(SARA_R5_MQTT_COMMAND) + strlen(topic) + SARA_R5_MQTT_TOPIC_EXTRA_MEMMORY + strlen(topic));
  if (command == NULL)
  {
	  // Return an error code if memory allocation fails
	  return SARA_R5_ERROR_OUT_OF_MEMORY;
  }
  
  // Construct the command to unsubscribe from the MQTT topic
  // The command is formatted with the MQTT unsubscribe command and the topic.
  sprintf(command, "%s=%d,\"%s\"\r", SARA_R5_MQTT_COMMAND, SARA_R5_MQTT_COMMAND_UNSUBSCRIBE, topic);
  
  // Send the command and check for the response
  // If the response is not OK, free the memory and return an error code.
  // Note: NULL and 0 are passed to avoid using a buffer for the response.
  if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, NULL,0,SARA_R5_STANDARD_RESPONSE_TIMEOUT)) // NULL i 0 per no posar un buffer
  {
	// Free the allocated memory and return an error code if the response is not as expected
	free (command);
	return SARA_R5_ERROR_ERROR;
  }

  // Free the allocated memory for the command string and return a success code
  free (command);
  return SARA_R5_ERROR_SUCCESS;
}

/**
 * Publishes a message to a specified MQTT topic.
 * @param topic The MQTT topic to publish the message to.
 * @param topicLength The length of the topic string.
 * @param buffer A memory area to store the response from the publish attempt.
 * @param size The size of the buffer in bytes.
 * @param QoS The Quality of Service level for the message.
 * @param retain Whether the message should be retained on the MQTT broker.
 * @param hex_mode Specifies if the message is in hex mode.
 * @param message The message to be published.
 * @param messageLength The length of the message.
 * @return Returns a success code if the message is successfully published, or an error code if the attempt fails.
 */
uint8_t saraR5PublishMQTT(const char *topic, uint8_t topicLength ,const char *buffer, int size, int QoS, int retain, uint8_t hex_mode, const uint8_t *message, uint8_t messageLength)
{
	char *command;
	int commandLength;

	// Validate input parameters
	if (topic == NULL || message == NULL || messageLength <= 0)
	{
		return SARA_R5_ERROR_UNEXPECTED_PARAM;
	}

	// Calculate the command length considering the MQTT publish command format
	// Additional characters are for commas and other command syntax elements.
	commandLength = strlen("AT+UMQTTC=2,") + QoS + retain + hex_mode + topicLength + messageLength + 3;
	command = saraR5CallocChar(commandLength);
	if (command == NULL)
	{
		// Return an error code if memory allocation fails
		return SARA_R5_ERROR_OUT_OF_MEMORY;
	}

	// Construct the command to publish the message
	// The command is formatted with the MQTT publish command, QoS, retain flag, hex mode, topic, and message.
	sprintf(command,"%s=%d,%d,%d,%d,\"%s\",\"%s\"\r", SARA_R5_MQTT_COMMAND, SARA_R5_MQTT_COMMAND_PUBLISH, QoS, retain, hex_mode , topic, message);

	// Send the command and check for the response
	// The response timeout is extended for publish operations.
	if(!saraR5SendCommandWithResponse(command, SARA_RESPONSE_OK, buffer, size,5 * SARA_R5_STANDARD_RESPONSE_TIMEOUT)) // NULL i 0 per no posar un buffer
	{
		// Free the allocated memory and return an error code if the response is not as expected
		free (command);
		return SARA_R5_ERROR_ERROR;
	}

	// Free the allocated memory for the command string and return a success code
	free (command);
	return SARA_R5_ERROR_SUCCESS;
}