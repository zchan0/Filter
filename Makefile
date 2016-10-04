CC		= g++
C		= cpp
H       = h

CFLAGS		= -g 
LFLAGS		= -g

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lOpenImageIO -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS   = -L /usr/lib64/ -lglut -lGL -lGLU -lOpenImageIO -lm
  endif
endif

HFILES  = ImageIO.${H}
OBJS    = ImageIO.o
PROJECT = filt

${PROJECT}:	${OBJS} ${PROJECT}.o
	${CC} ${LFLAGS} -o ${PROJECT} ${OBJS} ${PROJECT}.o ${LDFLAGS}

${PROJECT}.o: ${PROJECT}.${C} ${HFILES}
	${CC} ${CFLAGS} -c ${PROJECT}.${C}

ImageIO.o: ImageIO.${C} ${HFILES}
	${CC} ${CFLAGS} -c ImageIO.${C} 

clean:
	rm -f core.* *.o *~ ${PROJECT}
