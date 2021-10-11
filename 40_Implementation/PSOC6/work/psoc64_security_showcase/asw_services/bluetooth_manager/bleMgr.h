/****************************************************************************
* Filename        : bleMgr.h
* Author          : Hari Krishna Yelchuri
* Description     : Header file that declares functions for the Bluetooth
* 					Modules
*
****************************************************************************/

#ifndef ASW_SERVICES_BLUETOOTH_MANAGER_BLEMGR_H_
#define ASW_SERVICES_BLUETOOTH_MANAGER_BLEMGR_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "mbed.h"
#include <string>

/**
 * \brief Bluetooth Manager for providing services for configuring and
 * 		  connecting bluetooth modules.
 */
class bleMgr {

public:
	/**
	 * \brief Constructor for the bluetooth Manager
	 *
	 */
	bleMgr();
	/**
	 * \brief API to initialize the module as central node.
	 *
	 * @param module : Refers to the port used for the module.
	 */
	void initCentral(UARTSerial& module);
	/**
	 * \brief API to initialize the module as peripheral node.
	 *
	 * @param module : Refers to the port used for the module.
	 */
	void initPeripheral(UARTSerial& module);
	/**
	 * \brief API to command the module to start advertising itself.
	 *
	 * @param module : Refers to the port used for the module.
	 */
	void makeDiscoverable(UARTSerial& module);
	/**
	 * \brief API to discover the devices around the module.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 */
	void discoverDevices(UARTSerial& module);
	/**
	 * \brief API to connect to a selected module.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 */
	void tryConnect(UARTSerial& module);
	/**
	 * \brief API to write data over this UART port.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 * @param txData : Address to the data that has to be transmitted.
	 * @param length : Size of the data to be transmitted.
	 */
	void send(UARTSerial& module,const char *txData, unsigned int length);
	/**
	 * \brief API to receive data via UART port.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 * @param rxData : Address where the data to be received is stored.
	 * @param length : Size of the data to be received.
	 */
	int recieve(UARTSerial& module, char *rxData, unsigned int length);
	/**
	 * \brief API to receive 1 byte data over this UART port.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 */
	char recSingleChar(UARTSerial& module);
	/**
	 * \brief API to send acknowledgement for the data received.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 */
	void sendBleMgrAck(UARTSerial& module);
	/**
	 * \brief API to receive acknowledgement for the data sent.
	 *
	 * @param module : Refers to the port to which the module is connected.
	 */
	bool recBleMgrAck(UARTSerial& module);
};

#endif /* ASW_SERVICES_BLUETOOTH_MANAGER_BLEMGR_H_ */
