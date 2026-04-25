CC = gcc
LINKER = gcc

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -Wall -Wextra -O2

Q ?= @

SRC_FILES = main.c \
			tzdsp.c \
			nodes.c  \
			tzara.c \
			parser.c

OBJECTS = $(SRC_FILES:%.c=$(BUILD_DIR)/%.o)

.PHONY: clean install vimsyntax debug

default: tzara

debug:
	$(MAKE) clean
	$(MAKE) tzara CFLAGS="-Wall -Wextra -O0 -g" Q=

tzara: $(OBJECTS)
	$(Q)$(LINKER) -o $@ $^ -lm
	$(Q)echo "# Tzara Nodes" > nodes_list.md
	$(Q)echo " " >> nodes_list.md
	$(Q)./tzara --nodes >> nodes_list.md

$(OBJECTS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) tzara

install:
	cp tzara /usr/local/bin/tzara

format:
	clang-format -i src/main.c src/nodes.* src/parser.* src/tzara.* src/tzdsp.*

vimsyntax:
	@echo "Copying tzara syntax plugin to ~/.vim"
	@test -d ~/.vim/syntax || mkdir ~/.vim/syntax
	@test -d ~/.vim/ftdetect || mkdir ~/.vim/ftdetect
	@cp vim/syntax/tzara.vim ~/.vim/syntax/tzara.vim
	@cp vim/ftdetect/tzara.vim ~/.vim/ftdetect/tzara.vim

