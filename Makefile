NAME =	osuReplayPlayer

SRC =	main.c			\
		globals.c		\
		frame_buffer.c	\
		dict.c			\
		load_skin.c		\

OBJ =	$(SRC:%.c=src/%.o)

INC =	-Iinclude						\
		-Ilib/osureplay-parser/include	\
		-Ilib/osumap-parser/include		\
		-Ilib/concatf/include			\

CSFML = -lcsfml-audio		\
		-lcsfml-graphics	\
		-lcsfml-network		\
		-lcsfml-system		\
		-lcsfml-window		\


LDFLAGS =					\
	-L lib/osureplay-parser	\
	-losureplayparser		\
	-L lib/osumap-parser	\
	-losumapparser			\
	-L lib/concatf			\
	-lconcatf				\
	-lm

CFLAGS= $(INC)	\
	-W			\
	-Wall		\
	-Wextra		\

CC =	gcc

RULE =	all

LIBS =	lib/osureplay-parser/libosureplayparser.a	\
		lib/osumap-parser/libosumapparser.a			\
		lib/concatf/libconcatf.a					\

RES =

all:	$(LIBS) $(NAME)

lib/osureplay-parser/libosureplayparser.a:
	$(MAKE) -C lib/osureplay-parser $(RULE)

lib/osumap-parser/libosumapparser.a:
	$(MAKE) -C lib/osumap-parser $(RULE)

lib/concatf/libconcatf.a:
	$(MAKE) -C lib/concatf $(RULE)

$(NAME):$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS) $(CSFML) $(RES)

clean:
	$(MAKE) -C lib/osureplay-parser clean
	$(MAKE) -C lib/osumap-parser clean
	$(MAKE) -C lib/concatf clean
	$(RM) $(OBJ)
	$(RM) icon.res

fclean:	clean
	$(RM) $(NAME) $(NAME).exe

ffclean:fclean
	$(MAKE) -C lib/osureplay-parser fclean
	$(MAKE) -C lib/osumap-parser fclean
	$(MAKE) -C lib/concatf fclean

re:	fclean all

dbg:	CFLAGS += -g -O0
dbg:	RULE = dbg
dbg:	ffclean all
