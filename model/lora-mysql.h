
//remove this line if you do not want to use a mysql database to store packets.
/**
 * If database would be used, a few tables need to be implemented, they will be described here:
 *	-settings
 *     this database keeps track of the parameters send to the different devices. The fields are:
 *					- device (int)
 *					- datarate
 *					- power
 *					- channel
 *					- DRoffset
 *					- nbrep
 *					- dutycycle
 *					- RX1offset
 *					- RX2datarate
 *					- frequency
 *					- delay
 *  -uplinkdata
 *		 this table keeps track of all the packets sent to the base station, but also the snr, rssi and number of base stations receiving this packet
 *		 The fields in this table are:
 *					- time (could be real time or virtual time, default is now)
 *					- receiver
 *					- transmitter
 *					- channel
 *					- datarate
 *					- coderate
 *					- rssi
 *					- snr
 *					- size
 *					- data
 *					- framecount
 *					- receiving
 *	-whitelist
 *		this table lists all the devices whitelisted in network (ID), because this is simulated, also a field is added to whitelist multiple networks.
 *		The fields in this table are:
 *					- id
 *					- description
 *					- Xpos
 *					- Ypos
 *					- NetworkID (for which the device is whitelisted)
 *					- battery
 *					- margin
 */
//#define LORA_NETWORK_MYSQL 

#ifdef LORA_NETWORK_MYSQL
#define HOST "tcp://localhost:3306"
#define USER "lora"
#define PASS "lora"
#define DB "lora"
#endif
