TARGET = test_string_h
BUILD_DIR = build

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f $(ROOT_DIR)/$(TARGET)

$(BUILD_DIR)/%.o: %.c
	@ mkdir -p $(BUILD_DIR)
	gcc -g -c -o $@ $<

$(TARGET): $(OBJECTS)
	gcc -g $^ -o $@
	cp $@ $(ROOT_DIR)
