# Compilateur C++
CXX = g++

# Options de compilation
CXXFLAGS = -Wall -Wextra -std=c++11

# Librairies
LIBS = -lm -lrt

# Fichiers source et objet
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)

# Nom de l'exécutable
TARGET = mpu6050_reader

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
    
uninstall:
	rm -f /usr/local/bin/$(TARGET)
