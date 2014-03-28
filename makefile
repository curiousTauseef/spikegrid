ifeq ($(CC),cc)
	optflags= -Ofast -msse -msse2 -msse3 -funsafe-loop-optimizations -mtune=native -march=native  -floop-interchange -ftree-loop-optimize -floop-strip-mine -floop-block -flto  -fassociative-math -freciprocal-math -ffinite-math-only -fno-trapping-math
	extrawarnings=-Wstrict-aliasing -fstrict-aliasing   -Wshadow  -Wconversion -Wdouble-promotion -Wformat=2 -Wunused -Wuninitialized -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wcast-align -Wwrite-strings -Wjump-misses-init -Wlogical-op  -Wvector-operation-performance
	extraextrawarnings=-Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wstrict-overflow=5 
	export CFLAGS=-g -Wall -Wextra -std=gnu99 ${optflags} ${extrawarnings} ${extraextrawarnings}
else #clang
	export CFLAGS= -g -Wno-padded -Wno-missing-prototypes -Wno-missing-variable-declarations -Weverything -pedantic --std=gnu99 -Ofast

endif
export SPEEDFLAG=-DFAST #juse something else for double instead of float
export CLIBFLAGS= -fPIC -shared
export LDFLAGS=-lm -lpng
CFLAGS += ${SPEEDFLAG}
SOURCES= coupling.c  STDP.c conductance.c STD.c movie.c output.c evolve.c helpertypes.c
BINARY=a.out
VERSION_HASH = $(shell git rev-parse HEAD)
.PHONY: profile clean
${BINARY}: ${SOURCES}
	${CC}  ${CFLAGS} ${SOURCES} -o ${BINARY} ${LDFLAGS}
profile:
	${CC} ${CFLAGS} -pg ${SOURCES} -o ${BINARY} ${LDFLAGS}
time: ${BINARY}
	echo ${VERSION_HASH} $$( (/usr/bin/time  -f '%e' 'sh' '-c' './${BINARY} > /dev/null') 2>&1) >> times
	echo ${VERSION_HASH} $$( (/usr/bin/time  -f '%e' 'sh' '-c' './${BINARY} > /dev/null') 2>&1) >> times
	echo ${VERSION_HASH} $$( (/usr/bin/time  -f '%e' 'sh' '-c' './${BINARY} > /dev/null') 2>&1) >> times
	echo ${VERSION_HASH} $$( (/usr/bin/time  -f '%e' 'sh' '-c' './${BINARY} > /dev/null') 2>&1) >> times
	echo ${VERSION_HASH} $$( (/usr/bin/time  -f '%e' 'sh' '-c' './${BINARY} > /dev/null') 2>&1) >> times
	echo ${VERSION_HASH} $$( (/usr/bin/time  -f '%e' 'sh' '-c' './${BINARY} > /dev/null') 2>&1) >> times
clean:
	rm ${BINARY}
