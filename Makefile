OBJDIR = .
DESTDIR	= .
SRCDIR	= .
TEST?=recurrent_server
APPNAME=$(TEST)

TCH = /opt/fsl-imx-fb/3.14.52-1.1.0/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
CROSS_COMPILE = /opt/fsl-imx-fb/3.14.52-1.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
CFLAGS += -DLINUX -DEGL_API_FB -I$(TCH)/usr/include -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=$(TCH)
#system libraries
FSL_PLATFORM_LIB = $(TCH)/usr/lib

#other needed vars
ARCH = arm
CD = cd
DEL_FILE = rm -f
MKDIR = mkdir -p
RMDIR = rmdir
CC = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)g++
CP = cp
MAKE = make

LFLAGS = -L$(FSL_PLATFORM_LIB) --sysroot=$(TCH)
LFLAGS += -pthread -lEGL -lGLESv2  -lassimp -lIL -lm -lpng
SRCS = $(TEST).c

OBJECTS	= recurrent_server.o obj3d.o gUtil.o

first: all

all: $(APPNAME)

.PHONY: all

$(APPNAME): $(OBJECTS) 
	-@$(MKDIR) $(DESTDIR)
	$(LD) $(LFLAGS) -o $(APPNAME) $(OBJECTS)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(DEL_FILE) $(OBJECTS) $(UTILOBJS)
	$(DEL_FILE) *~ core *.core
	
distclean: clean
	$(DEL_FILE) $(APPNAME)
	-@$(RMDIR) $(DESTDIR)