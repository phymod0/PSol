
LDFLAGS += -lnfnetlink -lnetfilter_queue

C_FILES += $(wildcard *.c)
C_OFILES = $(patsubst %.c,%.o,$(C_FILES))
PROGRAM = psbox

$(PROGRAM): $(C_OFILES)
	$(CC) $(C_OFILES) -o psbox $(LDFLAGS)

$(C_OFILES):
	$(CC) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

clean:
	rm $(C_OFILES) $(PROGRAM)
