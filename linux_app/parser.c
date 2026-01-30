#include "parser.h"

// 這裡我們只做純邏輯判斷
int parse_packet(uint8_t *buffer, int length, BmcPacket *out_packet) {
    // 1. 檢查長度夠不夠
    if (length < sizeof(BmcPacket)) {
        return 0;
    }

    // 2. 檢查 Header
    if (buffer[0] != 0xAA) {
        return 0; // Header 錯誤
    }

    // 3. 填入結構 (Cast)
    BmcPacket *pkt = (BmcPacket*)buffer;

    // 4. 驗算 Checksum
    uint8_t calc_sum = pkt->header ^ pkt->cmd ^ pkt->len ^ pkt->data;

    if (calc_sum == pkt->checksum) {
        // 驗證成功，把資料複製出去
        *out_packet = *pkt;
        return 1; // Success
    } else {
        return 0; // Checksum Fail
    }
}