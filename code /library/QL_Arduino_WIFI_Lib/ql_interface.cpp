/***************************************************************************** 
* 
* File Name : ql_interface.c 
* 
* Description: qinglianyun`s interface function. 
* 
* Copyright (c) 2016 http://www.qinglianyun.com 
* All rights reserved. 
* 
* Author : ykgnaw
* 
* Date : 2016-06-27 
*****************************************************************************/ 
#include <stdio.h>
#include "ql_protocol.h"
#include "ql_queue.h"

static uint8_t ql_out_mem[MAX_WRITE_DATASIZE];
static uint8_t ql_in_mem[MAX_READ_DATASIZE];

struct RxCbHandler {
	void   (*query_status_cb)(int32_t);
    void   (*debug_cb)(uint32_t);
	void   (*pushmsg_cb)(int32_t);
	void   (*uploaddata_cb)(int32_t);
    void   (*gettime_cb)(uint32_t);
    int32_t (*respond_conf_cb)(const uint8_t*,uint8_t*,uint32_t);
    int32_t (*save_conf_cb)(const uint8_t*);
    int32_t (*handlecmd_cb)(uint8_t*,uint8_t*,uint8_t*);
    int32_t (*handlepush_cb)(uint8_t*,uint32_t);
	int32_t (*ota_binfile_chunk_cb)(otabin_filechunk*);
    int32_t (*ota_upgrade_cb)();
    int32_t (*send_bytes_cb)(uint8_t*,int32_t);
};
RxCbHandler rch;

QLCloud::QLCloud()
{
    init_queue(&g_RxFrameQUEUE);

    rch.query_status_cb =           NULL;
    rch.pushmsg_cb =                NULL;
    rch.uploaddata_cb =             NULL;
    rch.respond_conf_cb =           NULL;
    rch.save_conf_cb =              NULL;
    rch.gettime_cb =                NULL;
    rch.handlecmd_cb =              NULL;
    rch.handlepush_cb =             NULL;
    rch.debug_cb =                  NULL;
    rch.ota_binfile_chunk_cb =      NULL;
    rch.ota_upgrade_cb =            NULL;
    rch.send_bytes_cb =             NULL;
}

int32_t QLCloud::query_module_status(void)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_query_module_status_packet(ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);      
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
int32_t QLCloud::set_wifi_debug_status(int32_t open_or_close)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_is_open_wifi_debug_packet(open_or_close, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);      
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}

int32_t QLCloud::init(uint8_t *productid, uint8_t *productkey, uint8_t *mcu_version)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_initialization_packet(productid, productkey, mcu_version, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);      
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}

int32_t QLCloud::push_message(uint8_t *data, uint16_t len)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_pushmsg_packet(data, len, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
int32_t QLCloud::upload_data(uint8_t *key, uint16_t klen, uint8_t *value, uint16_t vlen)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_uploaddata_packet(key, klen, value, vlen, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
static int32_t upload_conf(uint8_t *conf, uint16_t conflen, uint32_t seq, int32_t errno)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_uploadconf_packet(conf, conflen, seq, errno, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
int32_t QLCloud::get_onlinetime()
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_getonlinetime_packet(ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
static int32_t save_conf_response(int32_t errno, uint32_t seq)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_responseconf_packet(errno, seq, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
static int32_t cmd_response(int32_t errno, uint8_t* res, uint32_t seq)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_responsecmd_packet(errno, res, seq, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}

static int32_t chunk_response(int32_t errno, uint32_t offset, uint32_t seq)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_otachunk_ack_packet(errno, offset, seq, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}
static int32_t upgrade_response(int32_t errno, uint32_t seq)
{
    if(rch.send_bytes_cb == NULL) {
        return QL_CALLBACK_NULL;
    }
    int32_t ret = 0;
    ret = assemble_upgrade_ack_packet(seq, errno, ql_out_mem, sizeof(ql_out_mem)); 
    if(ret > 0) {
        rch.send_bytes_cb(ql_out_mem, ret);
    } else {
        QL_DEBUG_PRINT("assemble save data error");
        return QL_MEM;
    }
}

void QLCloud::rx_byte(uint8_t byte)
{
    if( !is_full(&g_RxFrameQUEUE) ) {
        en_queue(&g_RxFrameQUEUE, byte);
	}
}
void QLCloud::reg_query_status_cb(void(*cb)(int32_t status))
{
	rch.query_status_cb = cb;
}
void QLCloud::reg_pushmsg_cb(void(*cb)(int32_t errcode))
{
	rch.pushmsg_cb = cb;
}
void QLCloud::reg_uploaddata_cb(void(*cb)(int32_t errcode))
{
	rch.uploaddata_cb = cb;
}
void QLCloud::reg_respond_conf_cb(int32_t(*cb)(const uint8_t *,uint8_t *,uint32_t))
{
    rch.respond_conf_cb = cb;
}
void QLCloud::reg_save_conf_cb(int32_t (*cb)(const uint8_t*))
{
    rch.save_conf_cb = cb;
}
void QLCloud::reg_gettime_cb(void(*cb)(uint32_t time))
{
	rch.gettime_cb = cb;
}
void QLCloud::reg_set_debug_status_cb(void(*cb)(uint32_t is_open))
{
	rch.debug_cb = cb;
}
void QLCloud::reg_handlecmd_cb(int32_t(*cb)(uint8_t*,uint8_t*,uint8_t*))
{
	rch.handlecmd_cb = cb;
}
void QLCloud::reg_handlepush_cb(int32_t(*cb)(uint8_t*,uint32_t))
{
	rch.handlepush_cb = cb;
}
void QLCloud::reg_ota_cb(int32_t (*chunk_cb)(otabin_filechunk*), int32_t (*upgrade_cb)(void))
{
	rch.ota_binfile_chunk_cb = chunk_cb;
	rch.ota_upgrade_cb       = upgrade_cb;
}

void QLCloud::reg_send_bytes_cb(int32_t (*cb)(uint8_t *,int32_t))
{
    rch.send_bytes_cb = cb;
}

void QLCloud::rx_data_handler()
{
    static int is_find_head = 0;
	static p_head_t frame_header;
    downloadcmd_info cmd_info;
    otabin_filechunk fchunk;
	uint8_t byte = 0;
	int i = 0;	
    int ret = 0;
    uint8_t tmp_data[MAX_CONF_SIZE];
	
	if( is_find_head == 0 && get_queue_len(&g_RxFrameQUEUE) >= sizeof(p_head_t) ) {
        get_queue_head(&g_RxFrameQUEUE, &byte);
		if(byte != 0xA5) {
    		de_queue(&g_RxFrameQUEUE, &byte);
			QL_DEBUG_PRINT("no magic\r\n");
			return;
        }
		for(i = 0; i < sizeof(p_head_t); i++) {
			de_queue(&g_RxFrameQUEUE, ql_in_mem+i);
		}
		
		parse_protocol_head( ql_in_mem, sizeof(p_head_t), &frame_header );
		if( frame_header.bodylen > 0) {
  			QL_DEBUG_PRINT("\r\n");
		    is_find_head = 1;
		} else {
  			QL_DEBUG_PRINT("no fd hd\r\n");
		    return;
		}
	}

	if(is_find_head) {
		if( get_queue_len(&g_RxFrameQUEUE) >= frame_header.bodylen) { //with crc length
			for(i = 0; i < frame_header.bodylen; i++) {
				de_queue(&g_RxFrameQUEUE, ql_in_mem+sizeof(p_head_t)+i);
			}
			p_body_format frame_body;
			parse_protocol_common_data( ql_in_mem, ql_in_mem+sizeof(p_head_t), frame_header.bodylen, &frame_body );
			
			if( frame_body.datalen >= 0 ) {
				switch(frame_body.type){
				case OP_TYPE_QUERY_STATUS:
					if(rch.query_status_cb!=NULL) {
						rch.query_status_cb(parse_module_status(&frame_body));
					}
					break;
				case OP_TYPE_DEBUG_STATUS:
					if(rch.debug_cb!=NULL) {
						rch.debug_cb(*((uint32_t *)frame_body.data));
					}
					break;
				case OP_TYPE_UPLOADDATA:
					if(rch.uploaddata_cb!=NULL) {
						rch.uploaddata_cb(*((uint32_t *)frame_body.data));
					}
					break;
				case OP_TYPE_UPLOADCONF:
                    memset(tmp_data, 0, MAX_CONF_SIZE);
                    if(rch.respond_conf_cb!=NULL) {
						ret = rch.respond_conf_cb((uint8_t*)frame_body.data, tmp_data, MAX_CONF_SIZE);
					} else {
                        ret = -1;
                    }
                    upload_conf(tmp_data, strlen((char*)tmp_data), frame_body.seq, ret);
                    break;
	            case OP_TYPE_DOWNLOADCONF:
                    if(rch.save_conf_cb!=NULL) {
						ret = rch.save_conf_cb((uint8_t*)frame_body.data);
					}else {
                        ret = -1;
                    }
                    save_conf_response(ret, frame_body.seq);
                    break;
				case OP_TYPE_PUSHMSG:
					if(rch.pushmsg_cb!=NULL) {
						rch.pushmsg_cb(*((uint32_t *)frame_body.data));
					}
					break;
				case OP_TYPE_RCV_PUSHMSG:
					if(rch.handlepush_cb!=NULL) {
						rch.handlepush_cb(frame_body.data, frame_body.datalen);
					}
					break;
				case OP_TYPE_GETONLINETIME:
					if(rch.gettime_cb!=NULL) {
						rch.gettime_cb(*((uint32_t *)frame_body.data));
					}
					break;
				case OP_TYPE_DOWNLOADCMD:
					parse_downloadcmd_info(&frame_body, &cmd_info);
                    if(cmd_info.key != NULL) {
    					if(rch.handlecmd_cb != NULL) {
    						ret = rch.handlecmd_cb(cmd_info.key, cmd_info.value, tmp_data);
    					}else {
                            ret = -1;
                        }
                    } else {
                        ret = -1;
                    }
                    cmd_response(ret, tmp_data, frame_body.seq);
					break; 
				case OP_TYPE_OTAFILE_CHUNK:
					parse_otafilechunk(&frame_body, &fchunk);
                    if(fchunk.data != NULL) {
    					if(rch.ota_binfile_chunk_cb != NULL){
    				   		ret = rch.ota_binfile_chunk_cb(&fchunk);
    					}else {
                            ret = -1;
                        }
                    } else {
                        ret = -1;
                    }
                    chunk_response(ret, fchunk.offset, frame_body.seq);
                    break;
				case OP_TYPE_OTAUPGRADE_CMD:
 					if(rch.ota_upgrade_cb != NULL){
				   		ret = rch.ota_upgrade_cb();
					} else {
					    ret = -1;
                    }
                    upgrade_response(ret, frame_body.seq);
                    break;
				default:
					QL_DEBUG_PRINT("unknown optype");
					break;
				}
				is_find_head = 0;
				memset(&frame_header, 0, sizeof(p_head_t));
				memset(&frame_body, 0, sizeof(p_body_format));
			} else {
				is_find_head = 0;
				memset(&frame_header, 0, sizeof(p_head_t));
				QL_DEBUG_PRINT("\r\nbody err!\r\n");
			}
		}
	}
}

