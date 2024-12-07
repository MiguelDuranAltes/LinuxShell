CC=gcc -Wall -g
HEADER_FILES_DIR =.
INCLUDES = -I $(HEADER_FILES_DIR)
OUTPUT = shell
LIB_HEADERS = -I $(HEADER_FILES_DIR)/ listaProg.h listaMem.h listaProc.h
SRCS = listaProg.c listaMem.c listaProc.c p3.c
OBJS = $(SRCS:.c=.o) 

#regla 1
$(OUTPUT): $(OBJS)
	$(CC) -o $(OUTPUT) $(OBJS) $(INCLUDES)
#regla 2
%.o: %.c $(LIB_HEADERS) 
	$(CC) -c –o $@ $<
#regla 3
cleanall: clean 
	rm -f $(OUTPUT) 
#regla 4
clean:
	rm -f *.o *~ 
