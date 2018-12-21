NAME =	osuReplayPlayer

SRC =	main.c					\

OBJ =	$(SRC:%.c=src/%.o)

INC =	-Iinclude			\
	-Ilib/osureplay-parser/include	\
	-Ilib/osumap-parser/include	\

CSFML = -lcsfml-audio		\
	-lcsfml-graphics	\
	-lcsfml-network		\
	-lcsfml-system		\
	-lcsfml-window		\


LDFLAGS =			\
	-L lib/osureplay-parser	\
	-losureplayparser	\
	-L lib/osumap-parser	\
	-losumapparser		\
	-llzma			\

CFLAGS= $(INC)			\
	-W			\
	-Wall			\
	-Wextra			\

CC =	gcc

RULE =	all

LIBS =	lib/osureplay-parser/libosureplay-parser.a	\
	lib/osumap-parser/osumap-parser.a		\

RES =

all:	$(LIBS) $(NAME)

lib/osureplay-parser/libosureplay-parser.a:
	$(MAKE) -C lib/osureplay-parser $(RULE)
	
lib/osumap-parser/osumap-parser.a:
	$(MAKE) -C lib/osumap-parser $(RULE)

$(NAME):$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS) $(CSFML) $(RES)

clean:
	$(MAKE) -C lib/osureplay-parser clean
	$(MAKE) -C lib/osumap-parser clean
	$(RM) $(OBJ)
	$(RM) icon.res

fclean:	clean
	$(RM) $(NAME) $(NAME).exe

ffclean:fclean
	$(MAKE) -C lib/osureplay-parser fclean
	$(MAKE) -C lib/osumap-parser fclean

re:	fclean all

dbg:	CFLAGS += -g -O0
dbg:	RULE = dbg
dbg:	ffclean all
