CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = BookHieve
SRCS     = main.cpp src/auth.cpp src/admin.cpp src/user_panel.cpp

# Detect OS for platform-specific commands
ifeq ($(OS),Windows_NT)
    EXE    = $(TARGET).exe
    RM     = del /f /q
    RUN    = .\$(TARGET).exe
else
    EXE    = $(TARGET)
    RM     = rm -f
    RUN    = ./$(TARGET)
endif

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	@echo "Build complete. Run with: $(RUN)"

clean:
	$(RM) $(EXE)

run: all
	$(RUN)
