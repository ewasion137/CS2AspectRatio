CC = gcc
CFLAGS = -Wall -shared
TARGET = CS2AspectRatio.dll

all:
	$(CC) $(CFLAGS) src/main.c -o $(TARGET) -luser32

clean:
	del /f $(TARGET)