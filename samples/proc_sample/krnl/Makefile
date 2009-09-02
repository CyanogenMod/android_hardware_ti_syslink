obj-m:=procmgr_app.o
EXTRA_CFLAGS := -I$(KRNLSRC)/arch/arm/plat-omap/include/
EXTRA_CFLAGS += -I$(KRNLSRC)/arch/arm/plat-omap/include/syslink/
EXTRA_CFLAGS += -I$(KRNLSRC)/drivers/dsp/syslink/procmgr/
EXTRA_CFLAGS += -I$(KRNLSRC)/drivers/dsp/syslink/procmgr/proc4430/
EXTRA_CFLAGS += -I$(PREFIX)/inc/

default:
	make -C $(KRNLSRC) M=$(PREFIX)/samples/proc_sample/krnl modules
clean:
	make -C $(KRNLSRC) M=$(PREFIX)/samples/proc_sample/krnl clean

install: procmgr_app.ko
	install -D procmgr_app.ko $(PREFIX)/target/syslink/procmgr_app.ko
