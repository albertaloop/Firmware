# Define default targets
.PHONY: all
all: main
	@echo "Building with CC=$(CC)"

# Check the value of ARCH and set CC accordingly
ifeq ($(ARCH),arm)
    ifndef CC
        $(error CC environment variable is not set)
    endif
    CC := $(CC)
else ifeq ($(ARCH),x86)
    CC := gcc
else
    $(error Invalid architecture: $(ARCH). Valid values are arm or x86.)
endif

# Compiler flags
CFLAGS := -Wall

# Source files
MAIN_SRC := main.c
PRIORITY_QUEUE_SRC := priority_queue.c
PRIORITY_QUEUE_HDR := priority_queue.h

# Object files
MAIN_OBJ := main.o
PRIORITY_QUEUE_OBJ := priority_queue.o

# Target to build the priority_queue object file
$(PRIORITY_QUEUE_OBJ): $(PRIORITY_QUEUE_SRC) $(PRIORITY_QUEUE_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# Target to build the main executable
.PHONY: main
main: $(MAIN_OBJ) $(PRIORITY_QUEUE_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# Target to build the main object file
$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
.PHONY: clean
clean:
	rm -f $(MAIN_OBJ) $(PRIORITY_QUEUE_OBJ) main