# Diretórios
SRC_DIR := src
CLIENT_SRC := $(SRC_DIR)/client
SERVER_SRC := $(SRC_DIR)/server
LIBS_SRC := $(SRC_DIR)/libs
TEST_DIR := testing
BIN_DIR := bin
BIN_CLIENT := $(BIN_DIR)/src
BIN_SERVER := $(BIN_DIR)/src
BIN_TEST := $(BIN_DIR)/testing

# Arquivos fonte
CLIENT_SOURCES := $(wildcard $(CLIENT_SRC)/*.cpp) $(wildcard $(LIBS_SRC)/*.cpp)
SERVER_SOURCES := $(wildcard $(SERVER_SRC)/*.cpp) $(wildcard $(LIBS_SRC)/*.cpp)
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.cpp) $(wildcard $(LIBS_SRC)/*.cpp)

# Objetos
OBJ_CLIENT := $(CLIENT_SOURCES:.cpp=.o)
OBJ_SERVER := $(SERVER_SOURCES:.cpp=.o)
OBJ_TEST := $(TEST_SOURCES:.cpp=.o)

# Compilador e flags
CXX := g++
CXXFLAGS := -Wall -std=c++20
DEBUGFLAGS := -g -DDEBUG
#RELEASEFLAGS := -O2

# Executáveis
CLIENT := $(BIN_CLIENT)/client
SERVER := $(BIN_SERVER)/server
#TEST := $(BIN_TEST)/test
TEST_CLIENT := $(BIN_TEST)/clientT
TEST_SERVER := $(BIN_TEST)/serverT

.PHONY: all client server test debug clean purge

all: CXXFLAGS += $(RELEASEFLAGS)
all: client server

test: test_client test_server

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: test_client test_server
#debug: client server

client: $(OBJ_CLIENT)
	@mkdir -p $(BIN_CLIENT)
	$(CXX) $(CXXFLAGS) $^ -o $(CLIENT)

server: $(OBJ_SERVER)
	@mkdir -p $(BIN_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $(SERVER)

#test: $(OBJ_TEST)
#	@mkdir -p $(BIN_TEST)
#	$(CXX) $(CXXFLAGS) $^ -o $(TEST)
test_client: testing/Client.cpp $(LIBS_SRC)/RawSocket.cpp $(LIBS_SRC)/KermitProtocol.cpp
	@mkdir -p $(BIN_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_CLIENT)

test_server: testing/Server.cpp $(LIBS_SRC)/RawSocket.cpp $(LIBS_SRC)/KermitProtocol.cpp
	@mkdir -p $(BIN_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_SERVER)


# Regra genérica para arquivos .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find $(SRC_DIR) $(TEST_DIR) -name "*.o" -delete
	rm -f $(CLIENT) $(SERVER) $(TEST)

purge: clean
	rm -rf $(BIN_DIR)


