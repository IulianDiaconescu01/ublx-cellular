# UBLX-CELLULAR

This project aims to develop a library to facilitate the use of the u-blox SARA R5 cellular connectivity module through UART communication. The library includes multiple functions that allow efficient handling of the module's communication and network configurations. Additionally, practical examples have been created to simplify the use and understanding of each available function.

## Contents

- **Main Library**: Implements key functions to interact with the SARA R5 module.
- **Examples**: Code samples for each function, helping to understand their practical use.

## Installation

1. Clone this repository:  
   ```bash
   git clone https://github.com/IulianDiaconescu01/ublx-cellular.git

2. Ensure your device's UART is configured to communicate with the SARA R5 board.

## Usage

1. Include the library in your project.

2. Review the examples starting with 01, 02, 03, etc., in the folder to learn how to use the functions as needed.

3. Compile and upload the code to your device.

## Examples

- **01.saraR5Comunication.c**: Checks communication with the SARA R5 module by sending an AT command to verify the connection and disable the echo. It then confirms whether the communication was successful based on the module's response.

- **02.saraR5NetworkInfo.c**: Initialises the microcontroller and peripherals, then retrieves and displays the Access Point Name (APN) and IP address associated with different contexts (up to three) of the SARA R5 module. It checks each context for a valid IP address and prints the corresponding APN and IP if found.

- **03.saraR5PDPaction.c**: It searches for available network operators using the SARA R5 module, displaying the details of each detected operator. It then performs a series of PDP (Packet Data Protocol) actions: disabling active profiles, loading a profile from non-volatile memory and activating the profile. If no operator is detected, the function stops with an error message indicating a network connection problem.

- **04.saraR5SocketSendUDP.c**: It performs several tasks with the SARA R5 module: first it retrieves and verifies the APN and IP address, then it activates a PDP context. It then opens a UDP socket, connects to a specified server, sends a ‘Hello, world!’ message, and finally closes the socket. If any step fails, the program stops with an error message.

- **05.saraR5PublishMQTT.c**: Initialises the SARA R5 module, retrieves APN and IP information, and activates a PDP context. It then attempts to disconnect any active MQTT connections, configures the MQTT client and server, and establishes a new MQTT connection. The function periodically posts to an MQTT topic every 20 seconds, stopping after five successful posts. If any step fails, the program stops with an error message.



