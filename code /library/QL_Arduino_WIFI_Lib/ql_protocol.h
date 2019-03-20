/*****************************************************************************
*
* File Name : ql_protocol.h
*
* Description: uart protocol.
*
* Copyright (c) 2016 http://www.qinglianyun.com
* All rights reserved.
*
* Author : ykgnaw
*
* Date : 2016-06-27
*****************************************************************************/

#ifndef __QL_PROTOCOL_H
#define __QL_PROTOCOL_H

#include <stdlib.h>

#include "Arduino.h"
#include "ql_interface.h"

#define QL_DEBUG_PRINT //Serial.println

#define MAX_BUF_SIZE            128

#define OPTYPE_LEN              2
#define DATA_FORMAT_HLEN        2
#define CRC_LEN                 2
#define SEQ_LEN                 4
#define ERRNO_LEN               4 

#define MAGIC_LABEL             0xA5

#define MAX_WRITE_DATASIZE      128
#define MAX_READ_DATASIZE       128
#define MAX_CONF_SIZE           128

enum {

    DATATYPE_JSON               = 1,
    DATATYPE_KLV                = 2,
    DATATYPE_BINARY             = 3,
    DATATYPE_QUERY              = 4,

    CLOUD_STREAM_TYPE =         0x01,
    CLOUD_HTTP_TYPE =           0x02,

    OP_TYPE_INIT                = 0x0001,
    OP_TYPE_QUERY_STATUS        = 0x0002,
    OP_TYPE_DEBUG_STATUS        = 0x0003,
    OP_TYPE_PUSHMSG             = 0x0401, 
    OP_TYPE_RCV_PUSHMSG         = 0x0402,
    OP_TYPE_UPLOADDATA          = 0x0501,
    OP_TYPE_UPLOADCONF          = 0x0601,
    OP_TYPE_DOWNLOADCONF        = 0x0701,
    OP_TYPE_DOWNLOADCMD         = 0x0801,
    OP_TYPE_GETONLINETIME       = 0x0901,
    OP_TYPE_OTAFILE_CHUNK       = 0x0a03,
    OP_TYPE_OTAUPGRADE_CMD      = 0X0a04,
};

typedef struct _p_head_t
{
    uint8_t	magic;
    uint8_t	type;
    uint16_t	bodylen;
} p_head_t;


typedef struct _p_body_format
{
    uint32_t seq;
    uint16_t type;
    int32_t datalen;
    uint8_t data[MAX_READ_DATASIZE];
} p_body_format;

typedef struct _downloadcmd_info
{
    uint8_t *key;
    uint16_t klen;
    uint8_t *value;
    uint16_t vlen;
} downloadcmd_info;

void assemble_p_header(uint8_t *header, int8_t type, int32_t len);
int32_t assemble_initialization_packet(uint8_t *productid, uint8_t *productkey, uint8_t *mcu_version, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_query_module_status_packet(uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_is_open_wifi_debug_packet(int is_open, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_pushmsg_packet(uint8_t *msg, uint16_t msglen, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_uploaddata_packet(uint8_t *key, uint16_t klen, uint8_t *value, uint16_t vlen, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_uploadconf_packet(uint8_t *conf, uint16_t conflen, int32_t errno, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_getonlinetime_packet(uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_responseconf_packet(int32_t errno, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_responsecmd_packet(int32_t errno, uint8_t* res, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_otachunk_ack_packet(int32_t errno, int32_t offset, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize);
int32_t assemble_upgrade_ack_packet(uint32_t seq, int32_t errno, uint8_t *outbuf, uint16_t outbufsize);
void parse_protocol_head(uint8_t *headptr, uint16_t headlen, p_head_t *header);
void parse_protocol_common_data(uint8_t* headptr, uint8_t* dataptr, uint16_t datalen, p_body_format *out_body);
void parse_downloadcmd_info(p_body_format *inbody, downloadcmd_info *cmdinfo);
void parse_otafilechunk(p_body_format *inbody,  otabin_filechunk *fchunk);
int32_t parse_module_status(p_body_format *inbody);

#endif
