#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

// 定義封包結構
typedef struct __attribute__((packed)) {
    uint8_t header;
    uint8_t cmd;
    uint8_t len;
    uint8_t data;
    uint8_t checksum;
} BmcPacket;

// 函式宣告：給我不完整的 buffer，我幫你檢查是不是有效封包
// 回傳值：1 = 成功, 0 = 失敗
int parse_packet(uint8_t *buffer, int length, BmcPacket *out_packet);

#endif