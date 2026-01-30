#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdint.h>
#include <signal.h>
#include "parser.h"

// <--- 2. 新增這個控制旗標
// volatile 代表「隨時可能被外部(中斷)改變」，告訴編譯器不要優化它
volatile sig_atomic_t keep_running = 1;

// <--- 3. 新增訊號處理函式 (Handler)
void handle_sigint(int sig) {
    (void)sig; // 消除 "unused parameter" 警告
    keep_running = 0; // 把旗標降下來，準備結束
    printf("\n\n>>> [System] Caught signal (Ctrl+C). Shutting down safely...\n");
}

// 設定 Serial Port 的魔法函式 (Linux 標準寫法)
int configure_serial(int fd) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, B115200); // 設定速度 115200
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;     // No Parity
    tty.c_cflag &= ~CSTOPB;     // 1 Stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         // 8 Bits
    tty.c_cflag &= ~CRTSCTS;    // No Hardware Flow Control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines

    // 重要：設定為 Raw Mode (讀什麼就是什麼，不要幫我處理換行符號)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable Software Flow Control
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes
    tty.c_oflag &= ~ONLCR; 

    // 設定讀取超時 (Blocking read)
    tty.c_cc[VTIME] = 10;    // Wait for up to 1s
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int main() {
    // 注意：如果你是用 WSL 透過 usbipd 連線，通常是 /dev/ttyACM0
    char *portname = "/dev/ttyACM0";
    int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }

    configure_serial(fd);
    printf("Serial Port %s Open! Waiting for STM32 data...\n", portname);

    // <--- 4. 註冊訊號 (告訴 OS：收到 SIGINT 時，去執行 handle_sigint)
    signal(SIGINT, handle_sigint);

    BmcPacket rx_packet;
    uint8_t buffer[100]; // 暫存區

    while (keep_running) {
        // 嘗試讀取資料 (一次讀一個封包的大小)
        int n = read(fd, buffer, sizeof(BmcPacket)); 
        
        if (n > 0) {
            // 簡單檢查 Header 是不是 0xAA
            if (parse_packet(buffer, n, &rx_packet)) {
             // 驗證成功！直接開始做事
                printf("[Valid] Temp: %d (Raw: %02X...)\n", rx_packet.data, buffer[0]);

                // --- 溫控邏輯 (這段不用動) ---
                char command = 0;
                if (rx_packet.data > 50) {
                    command = 'H';
                    printf("   >>> [Warning] Overheat! (H)\n");
                } else if (rx_packet.data < 20) {
                    command = 'L';
                    printf("   >>> [Info] Cool enough. (L)\n");
                }

                if (command != 0) {
                    write(fd, &command, 1);
                }
            } else {
                // 如果抓錯位置 (alignment error)
                printf("Syncing... Recv: %02X\n", buffer[0]);
            }
        }
    }
// <--- 6. 迴圈結束後的收尾工作
    printf("[System] Closing Serial Port...\n");
    close(fd);
    printf("[System] Bye!\n");

    return 0;
}