CXX=g++

CFLAGS=-g
LDFLAGS=-static 

ifeq ($(BIT), 64)
CFLAGS:=$(CFLAGS) -m64
LDFLAGS:=$(LDFLAGS) -shared -fPIC
TARGET:=libcomm_lib$(BIT).a
else
BIT:=32
CFLAGS:=$(CFLAGS) -m32
LDFLAGS:=$(LDFLAGS) -shared -fPIC -m elf_i386
TARGET:=libcomm_lib$(BIT).a
endif

INCLUDE=
LIBS= $(LDFLAGS)

BINARY = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all:$(TARGET)

%.o:%.cpp
	$(CXX) -c $< -o $@ $(INCLUDE) $(CFLAGS)

$(TARGET):$(BINARY)
	@rm -rf $@
	ar rcs $@ $^
	ln -s $@ libcomm.a

clean:
	@rm -rf ${BINARY} $(TARGET) libcomm.a

pkg:
	tar -czvf comm.tgz ../comm/*.h ../comm/*.cpp ../comm/*.txt ../comm/makefile*
	