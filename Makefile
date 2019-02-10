NAME =	osuReplayPlayer

SRC =	main.c					\
		globals.c				\
		frame_buffer.c			\
		dict.c					\
		load_skin.c				\
		bezier.c				\
		replay_player.c			\
		display_hit_object.c	\
		utils.c					\
		sound.c					\
		perfect_line.c			\
		get_slider_points.c		\
		encode_video.c			\
		encode_audio.c			\
		perfect_circle.c		\
		decode_audio_file.c		\

OBJ =	$(SRC:%.c=src/%.o)

INC =	-Iinclude						\
		-Ilib/libav-12.3				\
		-Ilib/concatf/include			\
		-Ilib/osumap-parser/include		\
		-Ilib/osureplay-parser/include	\

CSFML = -lcsfml-audio		\
		-lcsfml-graphics	\
		-lcsfml-network		\
		-lcsfml-system		\
		-lcsfml-window		\


LDFLAGS =							\
	-L lib/concatf					\
	-lconcatf						\
	-lpthread						\
	-L lib/osumap-parser			\
	-losumapparser					\
	-L lib/osureplay-parser			\
	-losureplayparser				\
	-L lib/libav-12.3/libavcodec/	\
	-lavcodec						\
	-L lib/libav-12.3/libavformat/	\
	-lavformat						\
	-L lib/libav-12.3/libavresample/\
	-lavresample					\
	-L lib/libav-12.3/libavutil/	\
	-lavutil						\
	-lm								\
	-lz

CFLAGS= $(INC)	\
	-W			\
	-Wall		\
	-Wextra		\

CC =	gcc

RULE =	all

LIBS =	lib/osureplay-parser/libosureplayparser.a	\
		lib/libav-12.3/libavcodec/libavcodec.a		\
		lib/osumap-parser/libosumapparser.a			\
		lib/concatf/libconcatf.a					\

RES =

all:	$(LIBS) $(NAME)

linux:	LDFLAGS += -lvdpau -lX11
linux:	all

lib/osureplay-parser/libosureplayparser.a:
	$(MAKE) -C lib/osureplay-parser $(RULE)

lib/osumap-parser/libosumapparser.a:
	$(MAKE) -C lib/osumap-parser $(RULE)

lib/concatf/libconcatf.a:
	$(MAKE) -C lib/concatf $(RULE)

lib/libav-12.3/libavcodec/libavcodec.a:
	chmod +x lib/libav-12.3/configure
	cd lib/libav-12.3 && ./configure && make

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
