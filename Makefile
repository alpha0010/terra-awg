# Config values

CFLAGS := -Wall -Wextra -pedantic -Werror -O2 -msse4.1 -mpclmul
CXXFLAGS := -Wall -Wextra -pedantic -Werror -std=c++20 -O2 -msse4.1 -mpclmul

SRCS := $(wildcard src/*.cpp) $(wildcard src/biomes/*.cpp) \
    $(wildcard src/biomes/doubleTrouble/*.cpp) \
    $(wildcard src/biomes/hardmode/*.cpp) \
    $(wildcard src/biomes/patches/*.cpp) \
    $(wildcard src/map/*.cpp) $(wildcard src/structures/*.cpp) \
    $(wildcard src/structures/hardmode/*.cpp) \
    $(wildcard src/structures/sunken/*.cpp) \
    $(wildcard src/structures/data/*.cpp) $(wildcard src/vendor/*.c) \
    $(wildcard src/vendor/*.cpp)
OUT := terra-awg

BUILD_DIR := build


# Build rules.

CPPFLAGS := -Isrc

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/$(OUT): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)

format:
	clang-format -i $(SRCS)

.PHONY: clean format
