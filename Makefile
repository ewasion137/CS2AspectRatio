CC = gcc
# -shared создает DLL
# -m64 указывает, что собираем под 64-бита (так как CS2 — это x64)
# -luser32 нужно, чтобы линковать MessageBox
CFLAGS = -shared -m64 
LIBS = -luser32
TARGET = CS2AspectRatio.dll

all:
	$(CC) $(CFLAGS) src/main.c -o $(TARGET) $(LIBS)

clean:
	del /f $(TARGET)