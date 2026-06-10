TARGET := ghash_demo
STEM := ghash

UV ?= uv
EXOCC ?= $(UV) run exocc
CC ?= cc
CFLAGS ?= -O3 -std=c11 -mpclmul -msse2

.PHONY: all generate check-codegen check benchmark clean

all: $(TARGET)

generate: $(STEM).c $(STEM).h

check-codegen: generate
	grep -q '_mm_clmulepi64_si128' $(STEM).c
	grep -q '_mm_shuffle_epi32' $(STEM).c

check: $(TARGET)
	./$(TARGET)

benchmark: check

$(TARGET): $(STEM).o main.o
	$(CC) $(CFLAGS) -o $@ $^

$(STEM).c: $(STEM).py
	$(EXOCC) -o . --stem $(STEM) $<

$(STEM).h: $(STEM).c

$(STEM).o: $(STEM).c $(STEM).h
	$(CC) $(CFLAGS) -c -o $@ $(STEM).c

main.o: main.c $(STEM).h
	$(CC) $(CFLAGS) -c -o $@ main.c

clean:
	$(RM) $(TARGET) $(STEM).c $(STEM).h $(STEM).d *.o
	$(RM) -r __pycache__
