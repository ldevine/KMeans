CXX      := g++
CXXFLAGS := -std=c++17 -O3
LDFLAGS  := -lm
INCLUDE  := -I.
SRC_DIR := KMeans
TARGET = KMeans

all: $(SRC_DIR)/$(TARGET).cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(TARGET) $(SRC_DIR)/$(TARGET).cpp $(LDFLAGS)

clean: 
	$(RM) $(TARGET)
	
