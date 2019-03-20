/***************************************************************************** 
* 
* File Name : ql_interface.h
* 
* Description: qinglianyun`s interface declaration. 
* 
* Copyright (c) 2016 http://www.qinglianyun.com 
* All rights reserved. 
* 
* Author : ykgnaw
* 
* Date : 2016-06-27 
*****************************************************************************/ 

#ifndef __QL_INTERFACE_H
#define __QL_INTERFACE_H

#include "Arduino.h"

/** define QL return code **/
#define QL_OK               0
#define QL_MEM	            -1
#define QL_CALLBACK_NULL	-2


#define QL_DEBUG_PRINT          Serial.println

enum MODULE_STATUS{
	STATUS_NOT_READY,
	STATUS_IS_READY,
	STATUS_NOT_INIT,
	STATUS_INITED,
	STATUS_NOT_CONNECT_CLOUD,
	STATUS_CONNECTED_CLOUD,
};

typedef struct _otabin_filechunk {
	uint8_t 	isend;
	uint32_t 	offset;
	int32_t 	datalen; 
	uint8_t 	*data;
} otabin_filechunk;

class QLCloud
{
public:
	/*
	*Function: constructor to initialize SDK`s queue and callbacks.
	*/
	QLCloud();
	/*
	*Function: send query module status packet to module.
	*     please register query status callback function to get the module status. 
	*return:
	*	QL return code
	*/
	int32_t query_module_status(void);
	/*
	*Function: open or close module debug info.
	*     please register set debug callback function to get the module debug status. 
	*parameters:
	*     is_open - (0) - close.
	*	               (1) - open.
	*return:
	*	QL return code
	*/
	int32_t set_wifi_debug_status(int32_t open_or_close);

	/**
	 * qlcloud_init - send initialize packet to cloud,initialize the contex that will be use to connect to cloud
	 *parameters:
	 * @product_id:
	 *      product_id are provided by cloud when products being created.
	 *      just copy the number on the web page
	 * @product_key:
	 *      product_key are provided by cloud, same as product_id
	 * @mcu_version:
	 *      version of the mcu program, used by OTA upgrade.
	 * Attention: this function must be called before the other functions in this file.
	 *
	 * return:
	 *      QL return code
	 */
	int32_t init(uint8_t *productid, uint8_t *productkey, uint8_t *mcu_version);

	/**
	 * qlcloud_push_message - push message to cloud, then cloud send it to the bound app
	 *parameters:
	 * data- message date
	 * len- length of data
	 * return:
	 *      QL return code

	 */
	int32_t push_message(uint8_t *data, uint16_t len);

	/*
	*Function: upload data to cloud
	*parameters:
	*	key - key  to upload.
	*	klen - key length.
	*     value -
	*     vlen - value length.
	*     outbuf - buffer used to store assembled packet.
	*     outbufsize - size of buffer.
	*return:
	*	QL return code
	*/
	int32_t upload_data(uint8_t *key, uint16_t klen, uint8_t *value, uint16_t vlen);

	/*
	*Function: send gettime packet to module.
	*parameters:
	*return:
	*	QL return code
	*/
	int32_t get_onlinetime();

	/*
	*Function: register query status callback.
	*parameters:
	*     cb - query status callback.
	*     status - mean the module current status, maybe with worth
			STATUS_NOT_READY - the wifi module is not ready
			STATUS_IS_READY  - the wifi module is ready to receive uart cmd.
			STATUS_NOT_INIT  - the wifi module is not init,you must call function 'assemble_initialization_packet' to assmble init packet, send to module to init it.
			STATUS_INITED    - the wifi module is initialized.
			STATUS_NOT_CONNECT_CLOUD - the wifi module is not connect the cloud.
			STATUS_CONNECTED_CLOUD - the wifi module is already connected the cloud.
	*return:
	*/
	void reg_query_status_cb(void(*cb)(int32_t status));
	/*
	*Function: register push message callback.
	*parameters:
	*     cb      - push message callback.
	*     errcode - the error code cloud return, maybe with worth
	*     0       - success 
	*     NZ      - failure
	*return:
	*/
	void reg_pushmsg_cb(void(*cb)(int32_t errcode));
	/*
	*Function: register upload data callback.
	*parameters:
	*     cb      - upload data callback.
	*     errcode - the error code cloud return, maybe with worth
	*     0       - success 
	*     NZ      - failure
	*return:
	*/
	void reg_uploaddata_cb(void(*cb)(int32_t errcode));
	/**
	 *Function:register upload config callback
	 *parameters:
	 *     cb      - upload config callback,you shuld upload mcu config in this funciton.
	 *     request_conf_key - the request key of config item.
	 *     response_conf - config send back to cloud,  buffer malloc by sdk, must not to malloc by User.
	 *     conf_max_size- maxsize of the third argument buffer, default maxsize is 512Byte.
	 */
	void reg_respond_conf_cb(int32_t(*cb)(const uint8_t *request_conf_key, uint8_t *response_conf, uint32_t conf_max_size));

	/**
	 *Function:register download config callback
	 *parameters:
	 *     cb        - download config callback
	 *     onfig_str - the newest config from app.
	 */
	void reg_save_conf_cb(int32_t (*cb)(const uint8_t* config_str));

	/*
	*Function: register get time callback.
	*parameters:
	*     cb   - get time callback.
	*     time - timestamp of current beijing time.
	*return:
	*/
	void reg_gettime_cb(void(*cb)(uint32_t time));
	/*
	*Function: register set debug status callback.
	*parameters:
	*     cb      - set debug status callback.
	*     errcode - the error code cloud return,maybe with worth
	*     0       - success
	*     1       - failure
	*return:
	*/
	void reg_set_debug_status_cb(void(*cb)(uint32_t status));
	/*
	*Function: register cmd handle callback.
	*parameters:
	*     cb    - cmd handle callback.
	*     key   - command,the content must be the same with the cloud`s command.
	*     value - data of command,the type is the same with the cloud`s type.
	*     response - data to response to cloud.
	*return:
	*/
	void reg_handlecmd_cb(int32_t(*cb)(uint8_t*key, uint8_t*value, uint8_t* response));
		/*
	*Function: register handle push message callback.
	*parameters:
	*     cb      - handle push message callback.
	*     pushmessage - push message from cloud
	*     pushmessage_len - push message length.
	*return:
	*/
	void reg_handlepush_cb(int32_t(*cb)(uint8_t*pushmessage,uint32_t pushmessage_len));
	/*
	*Function: register ota handle chunk and upgrade callback.
	*parameters:
	*     chunk_cb    - handle chunk packet callback.
	*     otabin_filechunk:
	*               offset- cur chunk data 's offset in the whole data.
	*               isend- cur chunk data is last part or not
	*               datalen- chunk data len.
	*               data- chunk data point.
	*     upgrade_cb    - handle upgrade packet callback.
	*return:
	*/
	void reg_ota_cb(int32_t (*chunk_cb)(otabin_filechunk *), int32_t (*upgrade_cb)(void));
	/*
	*Function: register send bytes callback.
	*     Attention:you must register this callback function before other all operation!
	*parameters:
	*     cb    - send bytes callback.
	*     data   - data to send.
	*     datalen - the data`s length.
	*return:
	*/
	void reg_send_bytes_cb(int32_t (*cb)(uint8_t *data,int32_t datalen));

	/*
	*Function: receive data from uart.this function must be called in function "serialEvent" or "loop" to receive uart`s data.
	*parameters:
	*     byte - byte the uart receive.
	*return:
	*/
	void rx_byte(uint8_t byte);

	/*
	*Function: receive and handle data function,this function must be called in "loop" function.
	*parameters:
	*return:
	*/
	void rx_data_handler();

};
#endif
