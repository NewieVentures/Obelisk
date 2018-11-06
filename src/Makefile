APP_DIR := app
OUTPUT_NAME := output
TEST_OUTPUT_NAME := tests

TEST_LIB := CppUTest CppUTestExt
TEST_LIB_DIRS := /usr/local/lib
TEST_DIR := test

TEST_SRC := colour.cpp utils.cpp ledStripDriver.cpp argParser.cpp cloudFunctions.cpp

CFLAGS := -g -std=c99 -Wall -Os -I$(TEST_DIR) -I$(APP_DIR)
CXXFLAGS := -g -std=c++11 -Wall -Os -I$(TEST_DIR) -I$(APP_DIR)

#*******************************************************************************
# Don't edit below this line!
#*******************************************************************************
SRC := $(shell find $(APP_DIR) -type f -iname '*.cpp' ! -iname ".*")

TEST_SRC := $(addprefix $(APP_DIR)/,$(TEST_SRC))
TEST_SRC += $(shell find $(TEST_DIR) -type f -iname '*.cpp')

TEST_LIBS := $(patsubst %,-l%,$(TEST_LIB))
TEST_LIB_PATHS := $(patsubst %,-L%,$(TEST_LIB_DIRS))

# determine the object files
OBJ := $(patsubst %.cpp,%.o, $(filter %.cpp,$(SRC)))
OBJ += $(patsubst %.c,%.o, $(filter %.c,$(SRC)))

TEST_OBJ := $(patsubst %.cpp,%.o, $(filter %.cpp,$(TEST_SRC)))
TEST_OBJ += $(patsubst %.c,%.o, $(filter %.c,$(TEST_SRC)))

all: $(TEST_OUTPUT_NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

# link the program
$(OUTPUT_NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(OUTPUT_NAME) $(OBJ) $(LIB_PATHS) $(LIBS)

$(TEST_OUTPUT_NAME): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_OUTPUT_NAME) $(TEST_OBJ) $(TEST_LIB_PATHS) $(TEST_LIBS)

clean:
	rm -f $(OUTPUT_NAME)
	rm -f $(TEST_OUTPUT_NAME)
	rm -f app/*.o
	rm -f $(TEST_DIR)/*.o
