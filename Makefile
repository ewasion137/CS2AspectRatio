CC = gcc
CFLAGS = -shared -m64 -I./include -I./minhook
TARGET = CS2AspectRatio.dll

# Собираем всё: наш файл, файлы MinHook и файлы внутри hde
SRC = src/main.c \
      minhook/buffer.c \
      minhook/hook.c \
      minhook/trampoline.c \
      minhook/hde/hde64.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -luser32 -lpsapi

clean:
	del /f $(TARGET)