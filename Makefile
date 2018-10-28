NAME =	osuReplayPlayer

SRC =	main.c					\

OBJ =	$(SRC:%.c=src/%.o)

INC =	-Iinclude			\
	-Ilib/osureplay-parser/include		\

CSFML = -lcsfml-audio		\
	-lcsfml-graphics	\
	-lcsfml-network		\
	-lcsfml-system		\
	-lcsfml-window		\


LDFLAGS =			\
	-L lib/osureplay-parser	\
	-losureplayparser	\
	-llzma			\

CFLAGS= $(INC)			\
	-W			\
	-Wall			\
	-Wextra			\

CC =	gcc

RULE =	all

LIBS =	lib/osureplay-parser/libosureplay-parser.a	\

RES =

all:	$(LIBS) $(NAME)

lib/osureplay-parser/libosureplay-parser.a:
	$(MAKE) -C lib/osureplay-parser $(RULE)

$(NAME):$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS) $(CSFML) $(RES)

clean:
	$(MAKE) -C lib/osureplay-parser clean
	$(RM) $(OBJ)
	$(RM) icon.res

fclean:	clean
	$(RM) $(NAME) $(NAME).exe

ffclean:fclean
	$(MAKE) -C lib/osureplay-parser fclean

re:	fclean all

dbg:	CFLAGS += -g -O0
dbg:	RULE = dbg
dbg:	ffclean all

epi:	CSFML = -lc_graph_prog
epi:	CFLAGS += -g -O0
epi:	RULE = dbg
epi:	re
