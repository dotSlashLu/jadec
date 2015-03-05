DIRS=src
OUT=*.log *.status *.cache configure config.h

configure:
	./configure.sh

clean:
	rm -rf $(OUT)
	for i in $(DIRS); do \
		(cd $$i && echo -e "\nCleaning $$i" && $(MAKE) clean) || exit 1; \
	done

test: configure
	cd src && $(MAKE) test

.PHONY: clean test
