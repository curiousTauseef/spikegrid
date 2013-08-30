export CFLAGS=-g -O3 -Wall -Wextra -std=gnu99 -lm 
export CLIBFLAGS= -fPIC -shared
SOURCES=parameters.c coupling.c  STDP.c conductance.c
BINARY=a.out
a.out: ${SOURCES}
	gcc  ${CFLAGS} ${SOURCES} -o ${BINARY}
clean:
	rm ${BINARY}
