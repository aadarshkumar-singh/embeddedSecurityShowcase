/****************************************************************************
* Filename        : bleMgr.cpp
* Author          : Hari Krishna Yelchuri
* Description     : Source file that declares functions for the bluetooth
* 					application
*
****************************************************************************/
#include "bleMgr.h"

/**
 * \brief Constructor for the bluetooth Manager
 *
 */
bleMgr::bleMgr()
{

}

/**
 * \brief API to initialize the module as central node.
 *
 * @param module : Refers to the port used for the module.
 */
void bleMgr::initCentral(UARTSerial &module) {

	module.write("AT+ROLE1",8);  //Set the device as a central (AT+ROLE1) or a peripheral (AT+ROLE0)
	module.write("AT+IMME1",8);  //Set the device as IMME1 will boot the device in command mode
								//else it will immediately start working
}

/**
 * \brief API to initialize the module as peripheral node.
 *
 * @param module : Refers to the port used for the module.
 */
void bleMgr::initPeripheral(UARTSerial &module) {

	module.write("AT+ROLE0",8);
	module.write("AT+IMME1",8);
}

/**
 * \brief API to command the module to start advertising itself.
 *
 * @param module : Refers to the port used for the module.
 */
void bleMgr::makeDiscoverable(UARTSerial &module) {

	module.write("AT+START",8); //Commands the device to start advertising itself.
}

/**
 * \brief API to discover the devices around the module.
 *
 * @param module : Refers to the port to which the module is connected.
 */
void bleMgr::discoverDevices(UARTSerial &module) {

	module.write("AT+DISC?",8); //Provides us information about the devices ready to pair nearby.
}

/**
 * \brief API to connect to a selected module.
 *
 * @param module : Refers to the port to which the module is connected.
 */
void bleMgr::tryConnect(UARTSerial &module) {

	//module.write(this->m_expArray,8);

}

/**
 * \brief API to write data over this UART port.
 *
 * @param module : Refers to the port to which the module is connected.
 * @param txData : Address to the data that has to be transmitted.
 * @param length : Size of the data to be transmitted.
 */
void bleMgr::send(UARTSerial &module, const char *txData, unsigned int length) {

	module.write(txData,length);

}

/**
 * \brief API to receive data via UART port.
 *
 * @param module : Refers to the port to which the module is connected.
 * @param rxData : Address where the data to be received is stored.
 * @param length : Size of the data to be received.
 */
int bleMgr::recieve(UARTSerial &module, char *rxData, unsigned int length) {

	int ret = module.read(rxData,length);
	if(ret != 0)
		return ret;
	else return 0;

}

/**
 * \brief API to receive 1 byte data over this UART port.
 *
 * @param module : Refers to the port to which the module is connected.
 */
char bleMgr::recSingleChar(UARTSerial &module) {

	char c[1];
	module.read(c,1);
	return c[0];

}

/**
 * \brief API to send acknowledgement for the data received.
 *
 * @param module : Refers to the port to which the module is connected.
 */
void bleMgr::sendBleMgrAck(UARTSerial& module) {

	module.write("**",2);
}

/**
 * \brief API to receive acknowledgement for the data sent.
 *
 * @param module : Refers to the port to which the module is connected.
 */
bool bleMgr::recBleMgrAck(UARTSerial& module) {

	char ack[2];

	module.read(ack,2);

	if(ack[0] == '*' && ack[1] == '*')
		return 1;
	else return 0;
}
