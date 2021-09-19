ifneq ($(KERNELRELEASE),)
obj-m := serial.o
else
KDIR := $(HOME)/Workspace/os/linux
all:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif
