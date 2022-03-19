LDFLAGS=-pthread

build:
	${CC} pthreadapp.c ${LDFLAGS} -o pthreadapp
