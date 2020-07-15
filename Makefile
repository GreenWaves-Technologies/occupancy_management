LIB_VER=1.1.0
export SHUTTERLESS_LIB=libshutterless.${LIB_VER}.a
export SHUTTERLESS_LIB_URL=.shutterless_url


all: gap8_project/shutterless/libshutterless.a

clean:
	rm -rf gap8_project/shutterless/lib*
	rm -f $(SHUTTERLESS_LIB_URL)

ifeq (,$(wildcard $(SHUTTERLESS_LIB_URL)))
$(SHUTTERLESS_LIB_URL): get_shutterless_lib.py
	python3 get_shutterless_lib.py
endif

gap8_project/shutterless/libshutterless.a: $(SHUTTERLESS_LIB_URL)
	#mkdir -p lib
	rm -rf gap8_project/shutterless/lib*
	./download_lib_shutterless.sh

.PHONY: all clean
