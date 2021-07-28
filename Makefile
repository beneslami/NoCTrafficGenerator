# Makefile
CPP    = g++ -std=gnu++0x -g
CPPFLAGS = -Wall -Wextra -pedantic -O2 -I. -Inetstream

PROG = tgen
PROGDIR :=
OBJDIR := obj
SUBDIRS = booksim/src

CPP_SRCS:=$(wildcard *.cpp)
NET_SRCS:=$(wildcard netstream/*.cpp)

OBJS :=  \
	$(CPP_SRCS:%.cpp=${OBJDIR}/%.o)\
	$(NET_SRCS:%.cpp=${OBJDIR}/%.o)\

#--- Make rules ---
all: $(PROG)

$(PROG): $(OBJS)
	$(CPP) $^ -o $(PROGDIR)$@

${OBJDIR}/%.o: %.cpp
	mkdir -p $(OBJDIR)/netstream
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm -f $(PROGDIR)$(PROG)
