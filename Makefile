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
INCLUDE := -Ilibs
INCLUDE_TEST := -Isrc/libs

# Arquivos fonte
CLIENT_SOURCES := $(wildcard $(CLIENT_SRC)/*.cpp) $(wildcard $(LIBS_SRC)/*.cpp)
SERVER_SOURCES := $(wildcard $(SERVER_SRC)/*.cpp) $(wildcard $(LIBS_SRC)/*.cpp)

# Para arquivo de Testes
TEST_CLIENT_SRC := testing/Client.cpp
TEST_SERVER_SRC := testing/Server.cpp
TEST_LIBS := $(wildcard $(LIBS_SRC)/*.cpp)

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
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $(CLIENT)

server: $(OBJ_SERVER)
	@mkdir -p $(BIN_SERVER)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $(SERVER)

test_client: $(TEST_CLIENT_SRC) $(TEST_LIBS)
	@mkdir -p $(BIN_TEST)
	$(CXX) $(CXXFLAGS) $(INCLUDE_TEST) $^ -o $(TEST_CLIENT)

test_server: $(TEST_SERVER_SRC) $(TEST_LIBS)
	@mkdir -p $(BIN_TEST)
	$(CXX) $(CXXFLAGS) $(INCLUDE_TEST) $^ -o $(TEST_SERVER)


# Regra genérica para arquivos .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find $(SRC_DIR) $(TEST_DIR) -name "*.o" -delete
	rm -f $(CLIENT) $(SERVER) $(TEST)

purge: clean
	rm -rf $(BIN_DIR)


