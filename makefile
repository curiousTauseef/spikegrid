optflags= -O3 -msse -msse2 -msse3 #-mtune=native -march=native 
extrawarnings=-Wstrict-aliasing -fstrict-aliasing   -Wshadow 
#these warnings only work on a really modern gcc - in particular they do not work on the default silliac / headnode compiler.  However, it is probably a good idea to enable them when you can
#extraextrawarnings=-Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wstrict-overflow=5
#Wconversion produces alot of wanrings about constants - particularly in Parameters.h 
extraextrawarnings=-Wconversion 
export CFLAGS=-g -Wall -Wextra -std=gnu99 ${optflags} ${extrawarnings}
export CLIBFLAGS= -fPIC -shared
export LDFLAGS=-lm -lpng
SOURCES= coupling.c  STDP.c conductance.c STD.c movie.c output.c evolve.c
BINARY=a.out
a.out: ${SOURCES}
	${CC}  ${CFLAGS} ${SOURCES} -o ${BINARY} ${LDFLAGS}
profile:
	${CC} ${CFLAGS} -pg ${SOURCES} -o ${BINARY} ${LDFLAGS}
clean:
	rm ${BINARY}
