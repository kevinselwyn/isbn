NAME    := isbn
EXEC    := bin/$(NAME)
FILES   := main.c $(NAME).c $(NAME)_meta.c
BIN_DIR := /usr/local/bin

all: build

setup:
	mkdir -p bin

build: setup cache_meta_barcode

bare: $(FILES)
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2

cache: $(FILES)
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_CACHING

meta: $(FILES)
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_META

barcode: $(FILES) $(NAME)_barcode.c
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_BARCODE

cache_meta: $(FILES)
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_CACHE -D_META

cache_barcode: $(FILES) $(NAME)_barcode.c
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_CACHE -D_BARCODE

meta_barcode: $(FILES) $(NAME)_barcode.c
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_META -D_BARCODE

cache_meta_barcode: $(FILES) $(NAME)_barcode.c
	gcc -Wall -Wextra -o $(EXEC) $^ -lm -lcurl -ljansson -lpng -O2 -D_CACHE -D_META -D_BARCODE

test: build
	./test/test.sh

install: $(EXEC)
	install -m 0755 $< $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

clean:
	rm -r -f $(EXEC) *.png bin/ cache/