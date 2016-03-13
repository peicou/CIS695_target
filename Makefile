OBJDIR=.
TEST?=recurrent_server
TARG=$(TEST)

TCH = /opt/fsl-imx-x11/3.14.52-1.1.0/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
CROSS_COMPILE = /opt/fsl-imx-x11/3.14.52-1.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
CFLAGS += -I$(TCH)/usr/src/kernel/include/uapi -I$(TCH)/usr/src/kernel/include -L$(TCH)/usr/lib -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=$(TCH)

all: $(TARG)

CC = $(CROSS_COMPILE)gcc
#LIBS += -lEGL -lGLESv2  -lX11 -lm
LIBS += -pthread

SRCS = $(TEST).c
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))


$(TARG):  $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Cleaning up...
	@rm *.o
	@rm $(TARG)
	@echo Done.
