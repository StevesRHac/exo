TARGET := pclmulqdq_reference_demo
STEM := pclmulqdq_reference

UV ?= uv
EXOCC ?= $(UV) run exocc
CC ?= cc
CFLAGS ?= -O2 -std=c11 -mpclmul

.PHONY: all generate check-codegen check clean

all: $(TARGET)

generate: $(STEM).c $(STEM).h

check-codegen: generate
	grep -q '_mm_clmulepi64_si128(a_reg, b_reg, 0x00)' $(STEM).c
	grep -q '_mm_clmulepi64_si128(a_reg, b_reg, 0x01)' $(STEM).c
	grep -q '_mm_clmulepi64_si128(a_reg, b_reg, 0x10)' $(STEM).c
	grep -q '_mm_clmulepi64_si128(a_reg, b_reg, 0x11)' $(STEM).c

check: $(TARGET)
	./$(TARGET)

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
