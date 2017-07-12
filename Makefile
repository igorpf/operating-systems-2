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
TEST_DIR=./teste
CFLAGS = -Wall
all: clean  directory library 

library: utils.o t2fs.o
	ar crs $(LIB_DIR)/libt2fs.a $(LIB_DIR)/*.o 
main: utils.o t2fs.o
	$(CC) $(CFLAGS) -o $(BIN_DIR)/main $(LIB_DIR)/*.o $(BIN_DIR)/*.o
directory:
	mkdir -pv $(BIN_DIR) $(TEST_DIR) 
t2fs.o: 
	$(CC) $(CFLAGS) -c $(SRC_DIR)/t2fs.c -o $(LIB_DIR)/t2fs.o 
utils.o: 
	$(CC) $(CFLAGS) -c $(SRC_DIR)/utils.c -o $(LIB_DIR)/utils.o 
clean:
	find  $(LIB_DIR) -type f ! -name 'apidisk.o' ! -name 'bitmap2.o' ! -name '*.c' ! -name 'Makefile' -delete



