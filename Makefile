CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses
TARGET = tui-notes
SRC = src/tui-notes.c

# Detect if running on Termux
ifeq ($(shell echo $(PREFIX)), /data/data/com.termux/files/usr)
    INSTALL_DIR = $(PREFIX)/bin
else
    INSTALL_DIR = $(HOME)/.local/bin
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f *~

install: $(TARGET)
	cp $(TARGET) $(INSTALL_DIR)/

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "Removed $(INSTALL_DIR)/$(TARGET)"

run: $(TARGET)
	./$(TARGET)

help:
	@echo "Available targets:"
	@echo "  all       - Build the application (default)"
	@echo "  debug     - Build with debugging symbols"
	@echo "  clean     - Remove compiled files"
	@echo "  install   - Install to Termux PREFIX/bin or ~/.local/bin/"
	@echo "  uninstall - Remove the application"
	@echo "  run       - Run the application"
	@echo "  help      - Show this message"

.PHONY: all debug clean install uninstall run help
