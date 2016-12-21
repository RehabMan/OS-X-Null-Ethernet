# really just some handy scripts...

KEXT=NullEthernet.kext
DIST=RehabMan-NullEthernet
BUILDDIR=./Build/Products

VERSION_ERA=$(shell ./print_version.sh)
ifeq "$(VERSION_ERA)" "10.10-"
	INSTDIR=/System/Library/Extensions
else
	INSTDIR=/Library/Extensions
endif

ifeq ($(findstring 32,$(BITS)),32)
OPTIONS:=$(OPTIONS) -arch i386
endif

ifeq ($(findstring 64,$(BITS)),64)
OPTIONS:=$(OPTIONS) -arch x86_64
endif

OPTIONS:=$(OPTIONS) -scheme NullEthernet

.PHONY: all
all: ssdt-rmne.aml
	xcodebuild build $(OPTIONS) -configuration Debug
	xcodebuild build $(OPTIONS) -configuration Release

.PHONY: clean
clean:
	xcodebuild clean $(OPTIONS) -configuration Debug
	xcodebuild clean $(OPTIONS) -configuration Release
	
.PHONY: update_kernelcache
update_kernelcache:
	sudo touch /System/Library/Extensions
	sudo kextcache -update-volume /

.PHONY: install_debug
install_debug:
	sudo cp -R $(BUILDDIR)/Debug/$(KEXT) $(INSTDIR)
	if [ "`which tag`" != "" ]; then sudo tag -a Purple $(INSTDIR)/$(KEXT); fi
	make update_kernelcache

.PHONY: install
install:
	sudo cp -R $(BUILDDIR)/Release/$(KEXT) $(INSTDIR)
	if [ "`which tag`" != "" ]; then sudo tag -a Blue $(INSTDIR)/$(KEXT); fi
	make update_kernelcache
	
.PHONY: install_inject
install_inject:
	sudo cp -R $(BUILDDIR)/Release/$(KEXT) $(INSTDIR)
	sudo cp -R $(BUILDDIR)/Release/NullEthernetInjector.kext $(INSTDIR)
	if [ "`which tag`" != "" ]; then sudo tag -a Blue $(INSTDIR)/$(KEXT); fi
	if [ "`which tag`" != "" ]; then sudo tag -a Blue $(INSTDIR)/NullEthernetInjector.kext; fi
	make update_kernelcache

.PHONY: install_force
install_force:
	sudo cp -R $(BUILDDIR)/Release/$(KEXT) $(INSTDIR)
	sudo cp -R $(BUILDDIR)/Release/NullEthernetForce.kext $(INSTDIR)
	if [ "`which tag`" != "" ]; then sudo tag -a Blue $(INSTDIR)/$(KEXT); fi
	if [ "`which tag`" != "" ]; then sudo tag -a Blue $(INSTDIR)/NullEthernetForce.kext; fi
	make update_kernelcache

.PHONY: distribute
distribute:
	if [ -e ./Distribute ]; then rm -r ./Distribute; fi
	mkdir ./Distribute
	cp -R $(BUILDDIR)/Debug ./Distribute
	cp -R $(BUILDDIR)/Release ./Distribute
	rm -Rf ./Distribute/Debug/NullEthernetInjector.kext
	rm -Rf ./Distribute/Debug/NullEthernetForce.kext
	rm -Rf ./Distribute/Release/NullEthernetForce.kext
	mv ./Distribute/Release/NullEthernetInjector.kext ./Distribute
	#mv ./Distribute/Release/NullEthernetForce.kext ./Distribute
	cp patch.txt ./Distribute
	cp ssdt-rmne.aml ./Distribute
	find ./Distribute -path *.DS_Store -delete
	find ./Distribute -path *.dSYM -exec echo rm -r {} \; >/tmp/org.voodoo.rm.dsym.sh
	chmod +x /tmp/org.voodoo.rm.dsym.sh
	/tmp/org.voodoo.rm.dsym.sh
	rm /tmp/org.voodoo.rm.dsym.sh
	ditto -c -k --sequesterRsrc --zlibCompressionLevel 9 ./Distribute ./Archive.zip
	mv ./Archive.zip ./Distribute/`date +$(DIST)-%Y-%m%d.zip`

ssdt-rmne.aml : ssdt-rmne.dsl
	iasl -p $@ $^

