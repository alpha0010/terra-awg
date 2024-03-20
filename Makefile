# Config values

CXXFLAGS := -Wall -Wextra -pedantic -Werror -std=c++20 -O2

SRCS := src/main.cpp src/Random.cpp src/World.cpp src/Writer.cpp \
    src/biomes/Forest.cpp
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
