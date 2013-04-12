OSTYPE := $(shell uname -s)

SRC_FILES = \
	main.cpp \
	kbhit.cpp \
	signal_catch.cpp\
	network_module.cpp\
	rapidxml-1.13/createXml.c

INC_DIRS += ../../../../../Samples/TrackPad
#INC_DIRS += ./rapidxml-1.13 

ifeq ("$(OSTYPE)","Darwin")
        LDFLAGS += -framework OpenGL -framework GLUT
else
        USED_LIBS += glut
endif

DEFINES += USE_GLUT

EXE_NAME = Kinect
include ../NiteSampleMakefile

