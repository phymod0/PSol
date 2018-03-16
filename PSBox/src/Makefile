
CC = gcc
LD = gcc

LDFLAGS += -lnfnetlink -lnetfilter_queue

SOURCE_FILES += $(wildcard *.c)
C_OFILES = $(patsubst %.c,%.o,$(SOURCE_FILES))

psbox: $(C_OFILES)
	$(LD) $(C_OFILES) -o psbox $(LDFLAGS)
	rm $(C_OFILES)

$(C_OFILES):
	$(CC) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

clean:
	rm psbox
