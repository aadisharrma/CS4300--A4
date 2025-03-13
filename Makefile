CXX      = g++
CXXFLAGS = -Wall -std=c++17 -I./include
LDFLAGS  = -L./lib
LIBS     = -lglad -lglfw3 -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL

# List of .cpp files for your main code (no glad.c here).
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
