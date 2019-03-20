/***************************************************************************** 
* 
* File Name : ql_protocol.cpp 
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

#include "ql_protocol.h"
#include "ql_queue.h"

uint16_t  STREAM_TO_UINT16_f(uint8_t* p, uint16_t offset)
{
    return ((uint16_t)((uint16_t)(*(p + offset)) << 8) + (uint16_t)(*(p + offset + 1)));
}

uint32_t STREAM_TO_UINT32_f(uint8_t* p, uint16_t offset)
{
    return  ((uint32_t)((uint32_t)
            (*(p + offset)) << 24) + (uint32_t)((uint32_t)
            (*(p + offset + 1)) << 16) + (uint32_t)((uint32_t)
            (*(p + offset + 2)) << 8) + (uint32_t)(*(p + offset + 3)));
}

uint8_t *UINT32_TO_STREAM_f (uint8_t *p, uint32_t u32)
{
    *(p)++ = (uint8_t)((u32) >> 24);
    *(p)++ = (uint8_t)((u32) >> 16);
    *(p)++ = (uint8_t)((u32) >> 8);
    *(p)++ = (uint8_t)(u32);
    
    return p;
}

uint8_t *UINT16_TO_STREAM_f (uint8_t *p, uint16_t u16)
{
    *(p)++ = (uint8_t)((u16) >> 8);
    *(p)++ = (uint8_t)(u16);
    
    return p;
}

static uint32_t send_seq = 0;
static uint32_t _get_cur_seq()
{
    return ++send_seq;
}

uint16_t wCRCTalbeAbs[] ={
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};

static uint16_t __crc16(uint8_t* pchMsg, uint16_t wDataLen, uint16_t precrc)
{
    uint16_t wCRC = precrc;
    uint16_t i = 0;
    uint8_t chChar;
    
    for (i = 0; i < wDataLen; i++) {
        chChar = *pchMsg++;
        wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
        wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
    }
    
    return wCRC;
}

static void _crc_tail(uint8_t *data, uint16_t datalen)
{
    uint16_t crcval = 0xffff;

    crcval = __crc16(data, datalen, crcval);
    
    UINT16_TO_STREAM_f(data+datalen, crcval);
}

void assemble_p_header(uint8_t *header, int8_t type, int32_t len)
{
    header[0] = MAGIC_LABEL;
    header[1] = type;
    UINT16_TO_STREAM_f(header+2, len + SEQ_LEN + OPTYPE_LEN);
}
/*
 *Function: assemble initialization packet with protocol format.
 *     this Function must be called when the module stataus is STATUS_NOT_INIT.
 *parameters:
 *     productid.
 *     productkey
 *     mcu_version
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *   (-1)  - err.
 *   (>0) - assembled packet real length, and data is in outbuf.
 */
int32_t assemble_initialization_packet(uint8_t *productid, uint8_t *productkey, uint8_t *mcu_version, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    uint8_t temp[128] = {0};
    int32_t len = 0;

    if (NULL == productid || NULL == productkey || NULL == mcu_version) {
        return -1;
    }

    len = (strlen((char *)productid)+strlen((char *)productkey)+strlen((char *)mcu_version)+2) < (sizeof(temp)-1) ? (strlen((char *)productid)+strlen((char *)productkey)+strlen((char *)mcu_version)+2) : sizeof(temp)-1;
    sprintf((char *)temp, "%s#%s#%s", (char *)productid, (char *)productkey, (char *)mcu_version);
    temp[strlen((char *)productid)] = 0x01;
    temp[strlen((char *)productid)+1+strlen((char *)productkey)] = 0x01;

    needsize = sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + len + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, len);
    
    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  _get_cur_seq());
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_INIT);
    
    memcpy(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN, temp, len);

    _crc_tail(outbuf, sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + len);

    return needsize;
}
/*
    *Function: assemble query module status packet with protocol format.
    *     this Function must be done first before all logic run
    *parameters:
    *     outbuf - buffer used to store assembled packet.
    *     outbufsize - size of buffer.
    *return:
    *   (-1)  - err.
    *   (>0) - assembled packet real length, and data is in outbuf.
    */
int32_t assemble_query_module_status_packet(uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    char* query_data = "GetModuleStatus";
    int query_len = strlen(query_data);
    
    needsize = sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + query_len + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, query_len);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  _get_cur_seq());
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_QUERY_STATUS);
    
    memcpy(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN,  query_data, query_len);

    _crc_tail(outbuf, sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + query_len);

    return needsize;
}

/*
    *Function: assemble packet to open or close module debug info.
    *parameters:
    *     is_open - (0) - close.
    *               (1) - open.
    *     outbuf - buffer used to store assembled packet.
    *     outbufsize - size of buffer.
    *return:
    *   (-1)  - err.
    *   (>0) - assembled packet real length, and data is in outbuf.
    */
int32_t assemble_is_open_wifi_debug_packet(int is_open, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    uint8_t sendmsg[4] = {0};
    
    needsize = sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + 4 + CRC_LEN;
    
    if (NULL == outbuf || outbufsize < needsize ) {
        return -1;
    }

    if( is_open != 0 && is_open != 1) {
        return -1;
    }
    
    UINT32_TO_STREAM_f(sendmsg, is_open);

    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, 4);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  _get_cur_seq());
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_DEBUG_STATUS);
    
    memcpy(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN,  sendmsg, 4);

    _crc_tail(outbuf, sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + 4);

    return needsize;
}
/*
 *Function: assemble pushmsg packet with protocol format.
 *parameters:
 *  msg - message info to send.
 *     msglen - length of message.
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err
 *  (>0) - assembled packet real length, and data is in outbuf.
 */
int32_t assemble_pushmsg_packet(uint8_t *msg, uint16_t msglen, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;

    needsize = sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + msglen + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, msglen);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  _get_cur_seq());
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_PUSHMSG);
    
    memcpy(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN,  msg, msglen);

    _crc_tail(outbuf, sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + msglen);

    return needsize;
}
/*
  *Function: assemble upload data packet with protocol format.
  *parameters:
  *   key - key  to save.
  *   klen - key length.
  *     value -
  *     vlen - value length.
  *     outbuf - buffer used to store assembled packet.
  *     outbufsize - size of buffer.
  *return:
  *   (-1)  - err.
  *   (>0) - assembled packet real length, and data is in outbuf.
  */
int32_t assemble_uploaddata_packet(uint8_t *key, uint16_t klen, uint8_t *value, uint16_t vlen, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    uint8_t *realdata = NULL;

    uint8_t sendmsg[MAX_WRITE_DATASIZE] = {0};
    int32_t msglen = 0;

    msglen = klen + vlen + 1;
    if (msglen >= MAX_WRITE_DATASIZE) {
        return -1;
    }

    memcpy(sendmsg, key, klen);
    *(sendmsg+klen) = '=';
    memcpy(sendmsg + klen + 1, value, vlen);
    
    needsize = sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + DATA_FORMAT_HLEN + msglen + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, msglen + DATA_FORMAT_HLEN);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  _get_cur_seq());
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_UPLOADDATA);

    realdata = outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN;
    *(realdata+0) = 0;
    *(realdata+1) = DATATYPE_QUERY;

    memcpy(realdata+DATA_FORMAT_HLEN, sendmsg,  msglen);

    _crc_tail(outbuf, sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + DATA_FORMAT_HLEN + msglen);
    
    return needsize;
}
/*
 *Function: assemble uploadconfigure packet with protocol format.
 *parameters:
 *  datatype-
        DATATYPE_JSON
        DATATYPE_BINARY
        DATATYPE_QUERY
 *  confdata - data info of configure.
 *     confdatalen - length of configure data.
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err.
 *  (>0) - assembled packet real length, and data is in outbuf.
 */
int32_t assemble_uploadconf_packet(uint8_t *conf, uint16_t conflen, int32_t errno, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;

    needsize = sizeof(p_head_t) + ERRNO_LEN+SEQ_LEN + OPTYPE_LEN + conflen + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, conflen+ERRNO_LEN);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  seq);
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_UPLOADCONF);
    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN, errno);
    
    memcpy(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN +ERRNO_LEN,  conf, conflen);

    _crc_tail(outbuf, sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN +ERRNO_LEN + conflen);

    return needsize;
}
/*
 *Function: assemble gettime packet with protocol format.
 *parameters:
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err.
 *  (>0) - assembled packet real length, and data is in outbuf.
 */
int32_t assemble_getonlinetime_packet(uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;

    needsize = sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + CRC_LEN;
    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, 0);
    
    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  _get_cur_seq());
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_GETONLINETIME);
    
    _crc_tail(outbuf, sizeof(p_head_t)+SEQ_LEN + OPTYPE_LEN);

    return needsize;
}
/*
 *Function: assemble response config packet with protocol format.
 *parameters:
 *     errno - errno
 *     seq - seq number, same worth with the matched command seq.
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err.
 *  (>0) - assembled packet real length, and data is in outbuf.
 */
int32_t assemble_responseconf_packet(int32_t errno, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    
    needsize = sizeof(p_head_t) + ERRNO_LEN+ SEQ_LEN + OPTYPE_LEN + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, ERRNO_LEN);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  seq);
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_DOWNLOADCONF);
    
    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN, errno);

    _crc_tail(outbuf, sizeof(p_head_t)+ERRNO_LEN+SEQ_LEN + OPTYPE_LEN);

    return needsize;
}
/*
 *Function: assemble response command packet with protocol format.
 *parameters:
 *     errno - errno
 *     seq - seq number, same worth with the matched command seq.
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err.
 *  (>0) - assembled packet real length, and data is in outbuf.
 */
int32_t assemble_responsecmd_packet(int32_t errno, uint8_t* res, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    uint32_t res_len = strlen((char *)res);
    needsize = sizeof(p_head_t) + ERRNO_LEN+ SEQ_LEN + OPTYPE_LEN + CRC_LEN +res_len;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, ERRNO_LEN +res_len);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  seq);
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_DOWNLOADCMD);
    
    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN, errno);
    memcpy(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN +ERRNO_LEN,  res, res_len);

    _crc_tail(outbuf, sizeof(p_head_t)+ERRNO_LEN+SEQ_LEN + OPTYPE_LEN +res_len);

    return needsize;
}
/*
 *Function: assemble ota chunk response packet.
 *parameters:
 *     errno - errno
 *     seq - seq number, same worth with the matched command seq.
 *     offset - the offset of otachunk.
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err.
 *  (>0) - assembled packet real length, and data is in outbuf.
 */

int32_t assemble_otachunk_ack_packet(int32_t errno, int32_t offset, uint32_t seq, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;
    
    needsize = sizeof(p_head_t) + 8 + SEQ_LEN + OPTYPE_LEN + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize) {
        return -1;
    }
    
    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, 8);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t),  seq);
    UINT16_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_OTAFILE_CHUNK);

    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN,  offset);
    UINT32_TO_STREAM_f(outbuf+sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN + 4,  errno);
    
    _crc_tail(outbuf, sizeof(p_head_t)+8+SEQ_LEN + OPTYPE_LEN);

    return needsize;
}
/*
 *Function: assemble upgrade response packet.
 *parameters:
 *     errno - errno
 *     seq - seq number, same worth with the matched command seq.
 *     outbuf - buffer used to store assembled packet.
 *     outbufsize - size of buffer.
 *return:
 *  (-1)  - err.
 *  (>0) - assembled packet real length, and data is in outbuf.
 */
 int32_t assemble_upgrade_ack_packet(uint32_t seq, int32_t errno, uint8_t *outbuf, uint16_t outbufsize)
{
    int32_t needsize = 0;

    needsize = sizeof(p_head_t) + 4 + SEQ_LEN + OPTYPE_LEN + CRC_LEN;

    if (NULL == outbuf || outbufsize < needsize)
    {
        return -1;
    }

    assemble_p_header(outbuf, CLOUD_STREAM_TYPE, 4);

    UINT32_TO_STREAM_f(outbuf + sizeof(p_head_t),  seq);
    UINT16_TO_STREAM_f(outbuf + sizeof(p_head_t) + SEQ_LEN,  OP_TYPE_OTAUPGRADE_CMD);

    UINT32_TO_STREAM_f(outbuf + sizeof(p_head_t) + SEQ_LEN + OPTYPE_LEN,  errno);

    _crc_tail(outbuf, sizeof(p_head_t) + 4 + SEQ_LEN + OPTYPE_LEN);

    return needsize;
}
static void _empty_header_format(p_head_t *hformat)
{
    if (hformat) {
        hformat->magic = 0;
        hformat->type = 0;
        hformat->bodylen = 0;
    }

    return;
}

void parse_protocol_head(uint8_t *headptr, uint16_t headlen, p_head_t *header)
{
    if (NULL == headptr || headlen < sizeof(p_head_t) || NULL == header) {
        return _empty_header_format(header);
    }
    
    header->magic = headptr[0];
    header->type = headptr[1];
    header->bodylen = STREAM_TO_UINT16_f(headptr, 2);

    if (header->magic != MAGIC_LABEL) {
        return _empty_header_format(header);
    }

    if (header->type != CLOUD_STREAM_TYPE && header->type != CLOUD_HTTP_TYPE) {
        return _empty_header_format(header);
    }

    header->bodylen += CRC_LEN;

    return;
}

static void _empty_body_format(p_body_format *bformat)
{
    if (bformat) {
        bformat->seq = 0;
        bformat->type = 0;
        bformat->datalen = 0;
        bformat->data[0] = '\0';
    }
    return;
}

void parse_protocol_common_data(uint8_t* headptr, uint8_t* dataptr, uint16_t datalen, p_body_format *out_body)
{
    uint16_t crcval = 0xffff;
    int32_t realdatalen = 0;
    uint8_t crcbuf[2] = {0};
    
    if (NULL == headptr || NULL == dataptr || datalen < SEQ_LEN+OPTYPE_LEN+CRC_LEN || NULL == out_body) {
        return _empty_body_format(out_body);
    }
    
    realdatalen = datalen - (SEQ_LEN+OPTYPE_LEN+CRC_LEN);
    if (MAX_READ_DATASIZE < realdatalen) {
        return _empty_body_format(out_body);
    }
    
    crcval = __crc16(headptr, sizeof(p_head_t), crcval);
    crcval = __crc16(dataptr, datalen-CRC_LEN, crcval);
    UINT16_TO_STREAM_f(crcbuf, crcval);

    if (memcmp(crcbuf, dataptr+datalen-CRC_LEN, CRC_LEN) !=0) {
        return _empty_body_format(out_body);
    }
    
    out_body->seq = STREAM_TO_UINT32_f(dataptr, 0);
    out_body->type = STREAM_TO_UINT16_f(dataptr, SEQ_LEN);
    
    out_body->datalen = realdatalen;
    memcpy(out_body->data, dataptr+SEQ_LEN+OPTYPE_LEN, realdatalen);

    return;
}

static void _empty_downloadcmd_format(downloadcmd_info *cmdinfo)
{
    if (cmdinfo) {
        cmdinfo->key = NULL;
        cmdinfo->klen = 0;
        cmdinfo->value = NULL;
        cmdinfo->vlen = 0;
    }
    return;
}
/*
*Function: parse protocol data to get download cmd data.
*parameters:
*     inbody - common body data.
*     struct _downloadcmd_info {
            uint8_t     *key;
            uint16_t    klen;
            uint8_t     *value;
            uint16_t    vlen;
        } downloadcmd_info;
*       2.if parse err, cmdinfo will be fixed by 0x00.
*return:
*/
void parse_downloadcmd_info(p_body_format *inbody, downloadcmd_info *cmdinfo)
{
    uint8_t *delimiter = NULL;
    uint8_t *ptr = NULL;
    uint8_t *end = NULL;
    
    if (NULL == inbody || inbody->datalen <= 0 || NULL == cmdinfo) {
        _empty_downloadcmd_format(cmdinfo);
        return;
    }

    if (inbody->type != OP_TYPE_DOWNLOADCMD) {
        _empty_downloadcmd_format(cmdinfo);
        return;
    }

    ptr = inbody->data;
    end = inbody->data + inbody->datalen;
    while (ptr < end) {
        if (*ptr == '=') {
            break;
        }
        ptr++;
    }

    if (ptr == inbody->data || ptr == end-1 || ptr == end) {
        _empty_downloadcmd_format(cmdinfo);
        return;
    }
    *ptr = '\0';
    delimiter = ptr;
    while (delimiter < end) {
        if (*delimiter == '&') {
            break;
        }
        delimiter++;
    }
    *(delimiter) = '\0';
    if (delimiter <= (ptr+1) ) {
        _empty_downloadcmd_format(cmdinfo);
        return;
    }

    cmdinfo->key = inbody->data;
    cmdinfo->klen = ptr - inbody->data;
    cmdinfo->value = ptr + 1;
    cmdinfo->vlen = (delimiter - (ptr+1));
    
    return;
}

static void _empty_filechunk_format(otabin_filechunk *fchunk)
{
    if (fchunk) {
        fchunk->isend= 0;
        fchunk->offset = 0;
        fchunk->datalen = 0;
        fchunk->data = NULL;
    }
    return;
}
/*
*Function: parse protocol data to get ota bin chunk data.
*parameters:
*     inbody - common body data.
*       1.if parse success,  body info will be stored in out_body.
            typedef struct _otabin_filechunk {
                uint8_t isend;
                uint32_t offset;
                int32_t datalen;
                uint8_t *data;
        } otabin_filechunk;
*       2.if parse err, fchunk will be fixed by 0x00.
*return:
*/
void parse_otafilechunk(p_body_format *inbody,  otabin_filechunk *fchunk)
{
    if (NULL == inbody || NULL == fchunk) {
        _empty_filechunk_format(fchunk);
        return;
    }

    if (inbody->type != OP_TYPE_OTAFILE_CHUNK) {
        _empty_filechunk_format(fchunk);
        return;
    }

    if (inbody->datalen <= 5) {
        _empty_filechunk_format(fchunk);
        return;
    }
    fchunk->isend = *(inbody->data+0);
    fchunk->offset = STREAM_TO_UINT32_f(inbody->data, 1);
    fchunk->datalen = inbody->datalen - 5;
    fchunk->data= inbody->data+5;

    return;
}
/*
*Function: parse protocol data to get module status.
*parameters:
*     inbody - common body data.
*return:
*    sucess:    STATUS_NOT_READY - the wifi module is not ready
*               STATUS_IS_READY  - the wifi module is ready to receive uart cmd.
*               STATUS_NOT_INIT  - the wifi module is not init,you must call function 'assemble_initialization_packet' to assmble init packet, send to module to init it.
*           STATUS_INITED    - the wifi module is initialized.
*               STATUS_NOT_CONNECT_CLOUD - the wifi module is not connect the cloud.
*               STATUS_CONNECTED_CLOUD - the wifi module is already connected the cloud.
*    fail: -1           
*/
int32_t parse_module_status(p_body_format *inbody)
{
    int status = -1;
    if (NULL == inbody) {
        return -1;
    }
    
    if(OP_TYPE_QUERY_STATUS != inbody->type) {
        return -1;
    }
    
    status = STREAM_TO_UINT16_f(inbody->data, 0);
    
    return status;
}

