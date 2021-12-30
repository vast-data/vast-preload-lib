all: 
	$(CXX) -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wl,-z,lazy -O3 -o ./test ./test.cpp
#	$(CXX) -Wl,-z,now -o ./test ./test.cpp
	
clean:
ifdef BUILD_VERBOSE
	rm -f ./test
else
	@echo "[DELETE] OBJECTS, DEPENDENCY_FILES, BINARIES"
	@rm -f ./test
endif

.PHONY: clean
