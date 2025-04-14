# Config values

CXXFLAGS := -Wall -Wextra -pedantic -Werror -std=c++20 -O2 -msse4.1 -mpclmul

SRCS := $(wildcard src/*.cpp) $(wildcard src/biomes/*.cpp) \
    $(wildcard src/map/*.cpp) $(wildcard src/structures/*.cpp) \
    $(wildcard src/structures/data/*.cpp) $(wildcard src/vendor/*.cpp)
OUT := terra-awg

BUILD_DIR := build


# Build rules.

CPPFLAGS := -Isrc

OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/$(OUT): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)

format:
	clang-format -i $(SRCS)

.PHONY: clean format
