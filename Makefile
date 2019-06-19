TARGET = template
OBJS = src/main.o data/cheetah.o data/forest.o

PSP_LARGE_MEMORY = 1

INCDIR = include
CFLAGS = -O3 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = libs
LDFLAGS =
LIBS = -lSDL2 -lGL -lGLU -lglut -lz -lpspvfpu -lpsphprm -lpspsdk -lpspctrl -lpspumd -lpsprtc \
       -lpsppower -lpspgum -lpspgu -lpspaudiolib -lpspaudio -lpsphttp -lpspssl -lpspwlan \
	   -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl -lm -lpspvram

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Template

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.bmp
	bin2o -i $< $@ $(addsuffix _bmp, $(basename $(notdir $<) ))

%.o: %.jpg
	bin2o -i $< $@ $(addsuffix _jpg, $(basename $(notdir $<) ))

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))
