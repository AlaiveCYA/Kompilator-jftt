CXXFLAGS = -Wall -g -std=c++17

# Katalog z plikami źródłowymi
SRC_DIR = src

BUILD_DIR = build

all: clean $(BUILD_DIR) kompilator

# Reguła tworząca katalog build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Reguła generująca plik nagłówkowy parser.tab.h oraz plik parser.tab.c
$(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h: | $(BUILD_DIR)
	bison -d -t -o $(BUILD_DIR)/parser.tab.c $(SRC_DIR)/parser.y

$(BUILD_DIR)/lex.yy.c: $(BUILD_DIR)/parser.tab.h | $(BUILD_DIR)
	flex -o $@ $(SRC_DIR)/lexer.l

kompilator: $(BUILD_DIR)/lex.yy.c $(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h
	$(CXX) $(CXXFLAGS) -I $(SRC_DIR) -o $@ $(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/lex.yy.c $(SRC_DIR)/*.cpp

# Reguła czyszczenia
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean