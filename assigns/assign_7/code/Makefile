# -*- makefile -*-

PROG =  bankdriver

LIB_SRC  = bankdriver.c teller.c branch.c bank.c account.c action.c debug.c report.c
DEPS = -MMD -MF $(@:.o=.d)

# XXX We probably want to provide them with debug and opt flags, since -O2
# might expose bugs which -O doesn't, but -O2 is hard to run through gdb.

WARNINGS = -pedantic -Wall -W -Wcast-qual -Wwrite-strings -Wextra -Wno-unused

# We need to use -std=gnu99 rather than -std=c99 because rand_r is not ANSI C.
CFLAGS += -fstack-protector -O -g -std=gnu99 -I$(INC_PATH) $(WARNINGS) $(DEPS)

LIB_OBJ = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(LIB_SRC)))
LIB_DEP = $(patsubst %.o,%.d,$(LIB_OBJ))
LIB = banklib.a

LIBS += -lpthread

PROG_SRC = bankdriver.c
PROG_OBJ = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(PROG_SRC)))
PROG_DEP = $(patsubst %.o,%.d,$(PROG_OBJ))

TMP_PATH := /usr/bin:$(PATH)
export PATH = $(TMP_PATH)


all: $(PROG)


$(PROG): $(PROG_OBJ) $(LIB)
	$(CC) $(LDFLAGS) $(PROG_OBJ) $(LIB) $(LIBS) -o $@

$(LIB): $(LIB_OBJ)
	rm -f $@
	ar r $@ $^
	ranlib $@

clean::
	rm -f $(PROG) $(PROG_OBJ) $(PROG_DEP)
	rm -f $(LIB) $(LIB_DEP) $(LIB_OBJ)

w4t1:
	make clean
	make
	chmod u+x bankdriver.o
	valgrind --tool=helgrind ./bankdriver -w4 -t1 -r

w1t1:
	make clean
	make
	chmod u+x bankdriver.o
	valgrind --tool=helgrind ./bankdriver -w1 -t1 -r

testAll:
	make clean
	make
	chmod u+x bankdriver.o
	./bankdriver -w4 -t1
	./bankdriver -w4 -t2
	./bankdriver -w4 -t3
	./bankdriver -w4 -t4
	./bankdriver -w4 -t5
	./bankdriver -w4 -t6
	./bankdriver -w4 -t7

.PHONY: all clean

-include $(LIB_DEP) $(PROG_DEP)
