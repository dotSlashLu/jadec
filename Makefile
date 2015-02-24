DIRS=src
OUT=*.log *.status *.cache configure config.h

clean:
	rm -rf $(OUT)
	for i in $(DIRS); do \
		(cd $$i && echo -e "\nCleaning $$i" && $(MAKE) clean) || exit 1; \
	done
