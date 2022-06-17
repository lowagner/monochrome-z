.ONESHELL:

run: Source
	directory=$(shell pwd)
	project=$${directory##*/}
	echo "Building project $$project"
	export PD_PROJECT_NAME=$$project
	#mkdir -p build && cd build && cmake .. && make && ${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $$project.so
	make -f PlaydateMakefile pdc && ${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $$project.pdx

debug: Source
	directory=$(shell pwd)
	project=$${directory##*/}
	export PD_PROJECT_NAME=$$project
	#mkdir -p buildDebug && cd buildDebug && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $$project.so
	make -f PlaydateMakefile debug && ${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $$project.pdx

Source:
	mkdir -p Source

clean:
	directory=$(shell pwd)
	project=$${directory##*/}
	export PD_PROJECT_NAME=$$project
	rm -rf build buildDebug && make -f PlaydateMakefile clean && rm src/test.h 

