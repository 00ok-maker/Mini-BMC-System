#include <stdio.h>
#include <string.h>
#include "parser.h"  // 引入我們要測試的大腦

// 這是我們自己寫的簡單測試工具
void run_test(const char* test_name, int expected_result, int actual_result) {
    if (expected_result == actual_result) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected %d, got %d\n", test_name, expected_result, actual_result);
    }
}

int main() {
    printf("Running Unit Tests for Protocol Parser...\n");
    printf("=========================================\n");

    BmcPacket packet;
    
    // --- 測試案例 1: 正常封包 ---
    // 模擬一個真實的資料: Header(AA) Cmd(01) Len(01) Data(32) Checksum(??)
    // Checksum = AA ^ 01 ^ 01 ^ 32 = 98
    uint8_t valid_data[] = {0xAA, 0x01, 0x01, 0x32, 0x98}; 
    
    int result = parse_packet(valid_data, 5, &packet);
    run_test("Test 1: Valid Packet", 1, result); // 我們預期回傳 1 (成功)
    
    if (result == 1) {
        // 加碼驗證解析出來的溫度對不對 (0x32 = 50度)
        run_test("Test 1b: Data Integrity", 50, packet.data);
    }

    // --- 測試案例 2: Checksum 錯誤 ---
    // 我們故意把最後一個 byte 改錯 (變成 0x99)
    uint8_t bad_checksum_data[] = {0xAA, 0x01, 0x01, 0x32, 0x99}; 
    
    result = parse_packet(bad_checksum_data, 5, &packet);
    run_test("Test 2: Bad Checksum", 0, result); // 我們預期回傳 0 (失敗)

    // --- 測試案例 3: Header 錯誤 ---
    // 開頭不是 AA (例如 BB)
    uint8_t bad_header_data[] = {0xBB, 0x01, 0x01, 0x32, 0x98};
    
    result = parse_packet(bad_header_data, 5, &packet);
    run_test("Test 3: Wrong Header", 0, result); // 我們預期回傳 0 (失敗)

    return 0;
}