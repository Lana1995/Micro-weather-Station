#include "os_type.h"
#include "osapi.h"
#include "user_interface.h"

#include "qlcloud_interface.h"

#define PRODUCTID  1000945640							//ע�⣡�������궨��һ��Ҫ�滻���ӹ�������ģ�
#define PRODUCTKEY "26214d3739f775796926797542e0a569"	//��Ȼ�п��ܵ����������ɹ�

ETSTimer task_timer;
#define  DATA         		"temperature = 100"
#define  PUSH_MSG      		"this is test push msg"

void ICACHE_FLASH_ATTR user_task(void* argv)
{
    static int k = 0;
    int time = 0;
    if(k == 0){
        //���ֻ���������Ϣ
        qlcloud_push_message(PUSH_MSG, os_strlen(PUSH_MSG));
        k = 1;
    } else if(k == 1) {
        //�������ݵ��ƶ�
        qlcloud_upload_data(SAVEDATA_TYPE_JSON, DATA, os_strlen(DATA));
        k = 0;
    }
}
/**
* ����״̬���ĵĻص�����
*/
void ICACHE_FLASH_ATTR on_conn_handle(int is_connected)
{
    os_printf("conn state:%d\r\n", is_connected);
    os_timer_disarm(&task_timer);//����״̬���ģ��ر��û�task
    if(is_connected) {//���ӳɹ��������û�task
        os_timer_setfn(&task_timer, (os_timer_func_t *)user_task, NULL);
        os_timer_arm(&task_timer, 2000, 1);
    }
}

/** 
 * �����ƶ��·�ָ��Ļص����� 
 * Ŀǰһ��ָ�����󳤶���2048Byte 
*/
int ICACHE_FLASH_ATTR on_downloadcmd_handle(download_cmdinfo *cmdinfo, char *out, int outsize)
{
    /* ��ȡ�ֻ��·������� */
    os_printf("Got command from cloud: %s = %s \r\n", cmdinfo->key, cmdinfo->value);

    /* ���ش����� */
    os_sprintf(out, "this is cmd reply");

    return ACK_OK;
}
/**
 * �����յ�push��Ϣ�Ļص�����
 */
void ICACHE_FLASH_ATTR on_recvpush_handle(char* push_msg, int msg_len)
{
    /* ��ȡ�ֻ����͵�push��Ϣ */
    os_printf("Get push message from cloud: %s, length:%d\r\n", push_msg, msg_len);
}
/**
 * �����ƶ˻�ȡ���õĻص����� 
 */
int ICACHE_FLASH_ATTR answer_getconfig_request_handle(char *request_conf_key, char *answerconf, int answerconf_size)
{
    /*��ȡ�ƶ��·������� */
    os_printf("Got a get config request from cloud. the config key is %s\r\n", request_conf_key);

    /*�ӱ��ػ�ȡkeyΪrequest_conf_key��������Ϣ*/

    os_sprintf(answerconf, "this is config answer");
    /*���ش����� */
    return ACK_OK;
}

/** 
 * ���±������õĻص����� 
*/
int ICACHE_FLASH_ATTR update_local_config_handle(char *config_str)
{
    /*��ȡ���µ�������Ϣ*/
    os_printf("Got a newest config from cloud:%s \r\n", config_str);
    /*���µ����ø��µ����� */
    return ACK_OK;
}
/** 
  **�ص�����������OTA���������� 
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
    
    /* ������ȡÿ��һ��chunk���ݰ��� �����ĸ���Ϣ:     
        ** 1���Ƿ������һ��chunk���ݿ�     
        ** 2����ǰchunk���ݿ������������е�ƫ��     
        ** 3����ǰchunk���ݿ�ĳ���     
        ** 4����ǰchunk���ݿ������     
        **/    
    os_printf("MCU: get_ota_binfile_chunk success.\r\n");    
    return ACK_OK;
}
/** 
  **�ص�����������OTA����ָ��
  **/
int process_ota_upgrade_command()
{    
    /* �����ƶ˷���������ָ��     
        ** ����MCU�����߼���������ǰ���յ���chunk���� 
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
        os_sprintf(config.password, "qinglian1234");   //����	
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
