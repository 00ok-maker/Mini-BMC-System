# 定義編譯器
CC = gcc
# 定義編譯參數 (-Wall 顯示警告, -g 方便除錯)
CFLAGS = -Wall -g

# 目標執行檔名稱
TARGET = serial_logger

# 需要編譯的原始碼檔案
SRCS = serial_logger.c parser.c

# 這是預設動作：打 'make' 就會執行這裡
all: $(TARGET)

# 編譯規則：把 .c 編譯成 執行檔
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# 清除規則：打 'make clean' 會把編譯出來的檔案刪掉
clean:
	rm -f $(TARGET)