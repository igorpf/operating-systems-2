#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
CFLAGS = -Wall
all: main

main: t2fs.o
	$(CC) $(CFLAGS) -o $(BIN_DIR)/main $(LIB_DIR)/*.o $(BIN_DIR)/t2fs.o

t2fs.o: 
	$(CC) $(CFLAGS) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o 
clean:
	find $(BIN_DIR) $(LIB_DIR) -type f ! -name 'apidisk.o' ! -name 'bitmap2.o' ! -name '*.c' ! -name 'Makefile' -delete



