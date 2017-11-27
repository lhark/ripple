# Source http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

SRCDIR := .
#LIBDIR := ../nas2d-core/build/lib
BUILDDIR := .
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/deps
#EXE := $(BINDIR)/OPHD
EXE := ripple

CFLAGS := -std=c++11 -g -fopenmp -Wall -Wno-unknown-pragmas -I/usr/include/glm -I/usr/include/eigen3 $(shell pkg-config --cflags glew)
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS := -fopenmp -lstdc++ -lm -lglfw -lGLEW -lGL
LDFLAGS += $(shell pkg-config --libs sdl2)

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

SRCS := $(shell find $(SRCDIR) -name '*.cpp')
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
OBJS := $(filter-out $(OBJDIR)/glm/% $(OBJDIR)/glew/% $(OBJDIR)/glfw/% $(OBJDIR)/ObjParser/Global% $(OBJDIR)/ObjParser/Main%,$(OBJS)) # Filter ui_builder
FOLDERS := $(sort $(dir $(SRCS)))

all: $(EXE)

$(EXE): $(OBJS)
	@mkdir -p ${@D}
	$(CXX) $^ $(LDFLAGS) -o $@

$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(DEPDIR)/%.d | build-folder
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

.PHONY:build-folder
build-folder:
	@mkdir -p $(patsubst $(SRCDIR)/%,$(OBJDIR)/%, $(FOLDERS))
	@mkdir -p $(patsubst $(SRCDIR)/%,$(DEPDIR)/%, $(FOLDERS))

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst $(SRCDIR)/%.cpp,$(DEPDIR)/%.d,$(SRCS)))

.PHONY:clean, clean-deps, clean-all
clean:
	-rm -fr $(OBJDIR)
	-rm -fr $(DEPDIR)
	-rm -f $(EXE)
clean-deps:
	-rm -fr $(DEPDIR)
clean-all:
	-rm -rf $(BUILDDIR)
