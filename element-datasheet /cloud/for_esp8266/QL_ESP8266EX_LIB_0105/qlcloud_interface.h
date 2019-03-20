#ifndef __QLCLOUD_INTERFACE_H__
#define __QLCLOUD_INTERFACE_H__

/** define return code **/
#define ACK_OK	0
#define ACK_ERR	-1

/**  do not change any macro definitions of ota logic as follows**/
#define FLASH_1M   1
#define FLASH_4M   2

#define EXT_FLASH_SIZE  FLASH_4M

#if (EXT_FLASH_SIZE == FLASH_1M)
  #define USER1BIN_LOC   0x01
  #define USER2BIN_LOC	 0x81
#elif (EXT_FLASH_SIZE == FLASH_4M)
  #define USER1BIN_LOC   0x01
  #define USER2BIN_LOC   0x101
#endif

#define OTA_CHUNK_SIZE  1024

/*
 * qlcloud_init - initialize the contex that will be use to connect to cloud
 * @product_id:
 * 		product_id are provided by cloud when products being created.
 * 		just copy the number on the web page
 * @product_key:
 * 		product_key are provided by cloud, same as product_id
 * @mcu_version:
 * 		version of the mcu program, used by OTA upgrade.
 * @device_macstr:
 * 		mac add with str format.
 * @flag:
 * 		crypt way, 0 - AES; 1-SSL
 *
 * Attention: this function must be called before the other functions in this file.
 *
 * return:
 * 		ACK_OK or ACK_ERR
 */
int qlcloud_initialization(unsigned int product_id,
				unsigned char *product_key,
				unsigned char *mcu_version,
				unsigned char is_debug_open);


typedef struct _download_cmdinfo {
	char *key;
	int klen;
	char *value;
	int vlen;
} download_cmdinfo;

/*
 * downloadcmd_cb - get download command info from cloud.
 * @download_cmdinfo:
 *		the content of command info.
 * @cmdresponse:
 * 		command result send back to cloud,  buffer malloc by sdk, must not to malloc by User.
 * @cmdresponse_size:
 * 		maxsize of the third argument buffer, default maxsize is 512Byte.
 * return:
 * 		ACK_OK or ACK_ERR
 */
typedef int (*downloadcmd_cb)(download_cmdinfo* cmdinfo, char *cmdresponse, int cmdresponse_size);

/*
 *  qlcloud_register_cmd_callback- register callback function to process command from cloud.
 */
void qlcloud_register_cmd_callback(downloadcmd_cb cmd_cb);

/*
 * receive_pushmessage_cb - get pushmessage info from cloud.
 * @pushmessage:
 * 		push message from cloud.
 * @pushmessage_len:
 * 		push message length.
 * return:
 */
typedef void (*receive_pushmessage_cb)(char *pushmessage, int pushmessage_len);

/*
 *  qlcloud_register_getpush_callback- register callback function to process pushmessage from cloud.
 */
void qlcloud_register_getpush_callback(receive_pushmessage_cb getpush_cb);

/*
 * answer_getconfig_request - answer the get config request from app.
 * @request_conf_key:
 *		the request key of config item.
 * @answerconf:
 * 		config send back to cloud,  buffer malloc by sdk, must not to malloc by User.
 * @cmdresponse_size:
 * 		maxsize of the third argument buffer, default maxsize is 512Byte.
 * return:
 * 		ACK_OK or ACK_ERR
 */
typedef int (*answer_getconfig_request)(char *request_conf_key, char *answerconf, int answerconf_size);
/*
 * qlcloud_status - cloud status.
 * @is_connected:
 *		1: connected.
 *           2: disconnected.
 * return:
 * 		
 */
typedef void (*qlcloud_status_cb)(int is_connected);
/*
 *  qlcloud_register_cloud_status- register callback functions to check device status is or not connected cloud.
 */
void qlcloud_register_status_callback(qlcloud_status_cb status_cb);

/*
 * update_local_config - update config which from cloud.
 * @config_str:
 * 		the newest config from app.
 * 	return:
 * 		ACK_OK or ACK_ERR
 */
typedef int (*update_local_config)(char *config_str);
/*
 *  qlcloud_register_conf_callbacks- register two callback functions to process config from cloud.
 */
void qlcloud_register_conf_callbacks(answer_getconfig_request answer_conf_cb, update_local_config updata_conf_cb);

/*
 * define content data format
 */
#define	SAVEDATA_TYPE_JSON		1
#define	SAVEDATA_TYPE_KLV		2 //unused.
#define	SAVEDATA_TYPE_BINARY	3
#define	SAVEDATA_TYPE_QUERY		4


/*
 * qlcloud_upload_data - upload data to cloud
 * @datatype:
 *  	SAVEDATA_TYPE_JSON
 *		SAVEDATA_TYPE_BINARY
 *		SAVEDATA_TYPE_QUERY -default.
 * @data: data to upload
 * @len: length of data
 * return null
 */
void qlcloud_upload_data(int datatype, char *data,  int len);


/*
 * cloud_push_message - push message to cloud, then cloud send it to the bound app
 * @data: message date
 * @len: length of data
 * return null
 */
void qlcloud_push_message(char* data, int len);

/*
 * qlcloud_get_onlinetime - return the timestamp that synchronize with cloud
 */
unsigned int qlcloud_get_onlinetime();

/********************** OTA Logic Part **************************/

typedef struct _binfile_chunk_data {
	char isend;
	unsigned int offset;
	int datalen;
	unsigned char *data;
} binfile_chunk_data;

/*
 * ota_binfile_chunk_cb - get the chunk data of ota upgrade bin file.
 * @binfile_chunk:
 * 	isend: cur chunk data is last part or not
 * 	offset: current chunk data's offset
 * 	datalen: chunk data len.
 * 	data: chunk data point.
 *
 * return:
 * 	ACK_OK or ACK_ERR
 */
typedef int (*ota_binfile_chunk_cb)(binfile_chunk_data *);

/*
 * ota_upgrade_cb - do real ota upgrade logic.
 * return:
 *	ACK_OK or ACK_ERR
 */
typedef int (*ota_upgrade_cb)();


/*
 *  qlcloud_register_ota_functions- register two callback functions for OTA upgrade logic.
 */
void qlcloud_register_ota_callbacks(ota_binfile_chunk_cb chunk_cb, ota_upgrade_cb cmd_cb);

#endif /* __INTERFACE_H__ */
