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
LIB_DIR=../lib
INC_DIR=../include
BIN_DIR=../bin
SRC_DIR=../src
TEST_DIR=../teste
CFLAGS = -Wall
all:  teste

teste: teste.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/teste teste.c  -L../lib -lt2fs
clean:
	rm -f $(BIN_DIR)/teste



