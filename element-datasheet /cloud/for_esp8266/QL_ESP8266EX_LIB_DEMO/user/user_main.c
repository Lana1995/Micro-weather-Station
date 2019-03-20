#include "os_type.h"
#include "osapi.h"
#include "user_interface.h"

#include "qlcloud_interface.h"

#define PRODUCTID  1000945640							//注意！这两个宏定义一定要替换您从官网申请的！
#define PRODUCTKEY "26214d3739f775796926797542e0a569"	//不然有可能导致联网不成功

ETSTimer task_timer;
#define  DATA         		"temperature = 100"
#define  PUSH_MSG      		"this is test push msg"

void ICACHE_FLASH_ATTR user_task(void* argv)
{
    static int k = 0;
    int time = 0;
    if(k == 0){
        //向手机端推送消息
        qlcloud_push_message(PUSH_MSG, os_strlen(PUSH_MSG));
        k = 1;
    } else if(k == 1) {
        //保存数据到云端
        qlcloud_upload_data(SAVEDATA_TYPE_JSON, DATA, os_strlen(DATA));
        k = 0;
    }
}
/**
* 连接状态更改的回调函数
*/
void ICACHE_FLASH_ATTR on_conn_handle(int is_connected)
{
    os_printf("conn state:%d\r\n", is_connected);
    os_timer_disarm(&task_timer);//连接状态更改，关闭用户task
    if(is_connected) {//连接成功，启动用户task
        os_timer_setfn(&task_timer, (os_timer_func_t *)user_task, NULL);
        os_timer_arm(&task_timer, 2000, 1);
    }
}

/** 
 * 处理云端下发指令的回调函数 
 * 目前一条指令的最大长度是2048Byte 
*/
int ICACHE_FLASH_ATTR on_downloadcmd_handle(download_cmdinfo *cmdinfo, char *out, int outsize)
{
    /* 获取手机下发的命令 */
    os_printf("Got command from cloud: %s = %s \r\n", cmdinfo->key, cmdinfo->value);

    /* 返回处理结果 */
    os_sprintf(out, "this is cmd reply");

    return ACK_OK;
}
/**
 * 处理收到push消息的回调函数
 */
void ICACHE_FLASH_ATTR on_recvpush_handle(char* push_msg, int msg_len)
{
    /* 获取手机推送的push消息 */
    os_printf("Get push message from cloud: %s, length:%d\r\n", push_msg, msg_len);
}
/**
 * 处理云端获取配置的回调函数 
 */
int ICACHE_FLASH_ATTR answer_getconfig_request_handle(char *request_conf_key, char *answerconf, int answerconf_size)
{
    /*获取云端下发的命令 */
    os_printf("Got a get config request from cloud. the config key is %s\r\n", request_conf_key);

    /*从本地获取key为request_conf_key的配置信息*/

    os_sprintf(answerconf, "this is config answer");
    /*返回处理结果 */
    return ACK_OK;
}

/** 
 * 更新本地配置的回调函数 
*/
int ICACHE_FLASH_ATTR update_local_config_handle(char *config_str)
{
    /*获取到新的配置信息*/
    os_printf("Got a newest config from cloud:%s \r\n", config_str);
    /*将新的配置更新到本地 */
    return ACK_OK;
}
/** 
  **回调函数，接收OTA升级包数据 
  **/
int process_ota_binfile_chunk(binfile_chunk_data *chunkinfo)
{    
    if (chunkinfo == NULL || chunkinfo->data == NULL || chunkinfo->datalen <= 0) {        
        os_printf("MCU: get_ota_binfile_chunk chunkinfo == null. \r\n");        
        return ACK_ERR;    
    }    
    os_printf("MCU: current chunk's offset = %d; datalen = %d.\r\n", chunkinfo->offset, chunkinfo->datalen);    

    if (chunkinfo->isend) {        
        os_printf("MCU: current chunk is last one.\r\n");    
    }   
    
    /* 处理收取每个一个chunk数据包， 包含四个信息:     
        ** 1、是否是最后一个chunk数据块     
        ** 2、当前chunk数据块在整个数据中的偏移     
        ** 3、当前chunk数据块的长度     
        ** 4、当前chunk数据块的数据     
        **/    
    os_printf("MCU: get_ota_binfile_chunk success.\r\n");    
    return ACK_OK;
}
/** 
  **回调函数，处理OTA升级指令
  **/
int process_ota_upgrade_command()
{    
    /* 处理云端发来的升级指令     
        ** 根据MCU自身逻辑，处理先前接收到的chunk数据 
        */    
    os_printf("\r\n MCU: get_ota_cmd success.\r\n");    
    return ACK_OK;
}

void ICACHE_FLASH_ATTR ql_init(void)
{
    os_printf("ql main here, %d\n", system_get_free_heap_size());
    
    qlcloud_initialization(PRODUCTID, PRODUCTKEY, "01.01", 1);

    qlcloud_register_cmd_callback(on_downloadcmd_handle);
    qlcloud_register_getpush_callback(on_recvpush_handle);
    qlcloud_register_conf_callbacks(answer_getconfig_request_handle, update_local_config_handle);
    qlcloud_register_ota_callbacks(process_ota_binfile_chunk, process_ota_upgrade_command);
    qlcloud_register_status_callback(on_conn_handle);
}

void ICACHE_FLASH_ATTR wifi_handle_event_cb(System_Event_t *evt)
{
    static uint8_t is_setup_wifi_first_run = 0;
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED:
			break;
		case EVENT_STAMODE_DISCONNECTED:
            os_printf("disconnect from ssid %s, reason %d\n",
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason);
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
            os_printf("mode: %d -> %d\n",
			evt->event_info.auth_change.old_mode,
			evt->event_info.auth_change.new_mode);
			break;
		case EVENT_STAMODE_GOT_IP:           
            if(is_setup_wifi_first_run == 0)
            {
                is_setup_wifi_first_run = 1;
                ql_init();
            }
			break;
		case EVENT_SOFTAPMODE_STACONNECTED:
            os_printf("station: " MACSTR "join, AID = %d\n",
			MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
            os_printf("station: " MACSTR "leave, AID = %d\n",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
			break;
		default:
			break;
		}
}

void ICACHE_FLASH_ATTR setup_wifi(void)
{
    struct station_config config;

	wifi_set_opmode(STATION_MODE);
    wifi_station_dhcpc_set_maxtry(5);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
    
    os_memset(&config, 0, sizeof(struct station_config));
    
    if (os_strlen(config.ssid) == 0) {    
        os_sprintf(config.ssid, "qltest");  //wifi ssid
        os_sprintf(config.password, "qinglian1234");   //密码	
    }
    
    os_printf("connect to ssid:%s,pw:%s\n", config.ssid, config.password);
    wifi_station_set_config(&config);
    wifi_station_connect();
}

void user_init(void)
{
    os_printf("SDK version:%s\n", system_get_sdk_version());
    
	system_init_done_cb(setup_wifi);
}
