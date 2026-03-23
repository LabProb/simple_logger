.PHONY: all build configure build_dir install run clean
BUILD_DIR=build
TARGET=Logger

all: build

configure:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..

build: configure
	$(MAKE) -C $(BUILD_DIR)

install: build
	# За замовчуванням встановлює в систему (можна задати DESTDIR або --prefix)
	cd $(BUILD_DIR) && cmake --install . --prefix $(PWD)/install

run: build
	./$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR) install

.DEFAULT_GOAL := all
