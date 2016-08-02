CFLAGS := -g -Wall -Werror -Ilib
BIN := jash

SRCDIR = src
BLDDIR = build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)%.c,$(BLDDIR)%.o, $(SRCS))

.PHONY: all
all: $(BIN)

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BLDDIR) $(BIN)
