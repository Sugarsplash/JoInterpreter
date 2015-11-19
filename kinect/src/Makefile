include ./Common/CommonDefs.mak

BIN_DIR = ./

INC_DIRS = \
	../Include \
	./

SRC_FILES = ./*.cpp

ifeq ("$(OSTYPE)","Darwin")
	LDFLAGS += -framework OpenGL -framework GLUT
else
	USED_LIBS += glut GL
endif

USED_LIBS += OpenNI

EXE_NAME = JoInterpreter

include ./Common/CommonCppMakefile

