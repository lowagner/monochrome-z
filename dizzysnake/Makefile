.ONESHELL:
.PHONY: run
# TODO: come up with a better way to share libs between CMakeLists.txt and here.
%: export PD_PROJECT_LIBS=
%: export PD_PROJECT_MODES=snake
%: export PD_PROJECT_NAME=$(shell basename $(shell pwd))

run: Source
	@echo "===================================="
	@echo "Building project $${PD_PROJECT_NAME}"
	@echo "===================================="
	mkdir -p $${PD_PROJECT_NAME}.pdx
	touch $${PD_PROJECT_NAME}.pdx/pdxinfo
	cp $${PD_PROJECT_NAME}.pdx/pdxinfo bakup-pdxinfo
	make -f ../PlaydateMakefile pdc && $${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $${PD_PROJECT_NAME}.pdx
	mv bakup-pdxinfo $${PD_PROJECT_NAME}.pdx/pdxinfo

device: Source
	@echo "===================================="
	@echo "Building pdx for $${PD_PROJECT_NAME}"
	@echo "===================================="
	# NOTE: required adding --specs=nosys.specs after $(CC) to buildsupport/common.mk pdex.elf rule
	make -f ../PlaydateMakefile
	zip -r new.zip $${PD_PROJECT_NAME}.pdx
	mv new.zip $${PD_PROJECT_NAME}.zip

debug: Source
	@echo "=========================================="
	@echo "Building debug project $${PD_PROJECT_NAME}"
	@echo "=========================================="
	make -f ../PlaydateMakefile debug && $${PLAYDATE_SDK_PATH}/bin/PlaydateSimulator $${PD_PROJECT_NAME}.pdx

clean:
	rm -rf build buildDebug
	# make -f ../PlaydateMakefile clean
	rm -rf core mode

Source:
	mkdir -p Source
