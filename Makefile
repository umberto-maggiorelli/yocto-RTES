LDFLAGS=-pthread

build:
	${CC} corsa.c ${LDFLAGS} -o corsa
