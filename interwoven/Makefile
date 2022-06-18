.ONESHELL:
.PHONY: run
# TODO: come up with a better way to share libs between CMakeLists.txt and here.
%: export PD_PROJECT_LIBS=data display font playdate runtime
%: export PD_PROJECT_NAME=$(shell basename $(shell pwd))

run: Source
	@echo "===================================="
	@echo "Building project $${PD_PROJECT_NAME}"
	@echo "===================================="
	#mkdir -p build && cd build && cmake .. && make && ${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $$project.so
	make -f ../PlaydateMakefile pdc && $${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $${PD_PROJECT_NAME}.pdx

debug: Source
	@echo "=========================================="
	@echo "Building debug project $${PD_PROJECT_NAME}"
	@echo "=========================================="
	#mkdir -p buildDebug && cd buildDebug && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $$project.so
	make -f ../PlaydateMakefile debug && $${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $${PD_PROJECT_NAME}.pdx

clean:
	rm -rf build buildDebug && make -f ../PlaydateMakefile clean && rm src/test.h 

Source:
	mkdir -p Source
