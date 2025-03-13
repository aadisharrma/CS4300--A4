OS := $(shell uname 2>/dev/null || echo Unknown)

CXX      = g++
CXXFLAGS = -Wall -std=c++17
LDFLAGS  =
LIBS     =

# OS-specific flags
ifeq ($(OS), Darwin)
    CXXFLAGS += -I./include
    LDFLAGS  += -L./lib
    LIBS     += -lglad -lglfw3 -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL
else ifeq ($(OS), Linux)
    CXXFLAGS += -I./include
    LDFLAGS  += -L./lib
    LIBS     += -lglad -lglfw -lGL -ldl -lpthread
else
    CXXFLAGS += -I./include
    LDFLAGS  += -L./lib
    LIBS     += -lglad -lglfw3 -lopengl32 -lgdi32
endif

SRCS = main.cpp \
       DroneController.cpp \
       DroneModel.cpp \
       DroneView.cpp \
       ShaderProgram.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = drone

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
