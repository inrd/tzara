CC = gcc
LINKER = gcc

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -Wall -Wextra -O2

SRC_FILES = main.c \
			nodes.c  \
			tzara.c \
			parser.c

OBJECTS = $(SRC_FILES:%.c=$(BUILD_DIR)/%.o)

.PHONY: clean install

default: tzara

tzara: $(OBJECTS)
	$(LINKER) -o $@ $^ -lm

$(OBJECTS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) tzara

install:
	cp tzara /usr/local/bin/tzara

