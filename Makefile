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

.PHONY: clean install vimsyntax

default: tzara

tzara: $(OBJECTS)
	@$(LINKER) -o $@ $^ -lm
	@echo "# Tzara Nodes" > nodes_list.md
	@echo " " >> nodes_list.md
	@./tzara --nodes >> nodes_list.md

$(OBJECTS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) tzara

install:
	cp tzara /usr/local/bin/tzara

vimsyntax:
	@echo "Copying tzara syntax plugin to ~/.vim"
	@test -d ~/.vim/syntax || mkdir ~/.vim/syntax
	@test -d ~/.vim/ftdetect || mkdir ~/.vim/ftdetect
	@cp vim/syntax/tzara.vim ~/.vim/syntax/tzara.vim
	@cp vim/ftdetect/tzara.vim ~/.vim/ftdetect/tzara.vim

