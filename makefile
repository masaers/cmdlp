
CXXFLAGS+=-Wall -pedantic -std=c++11 -g -O3
LDFLAGS=

BIN_NAMES=
TEST_NAMES=cmdlp_test

OBJECTS=$((filter-out $(BIN_NAMES:%=%.cpp),$(wildcard *.cpp)):%.cpp=%.o) \
        $((filter-out $(TEST_NAMES:%=%.cpp),$(wildcard *.cpp)):%.cpp=%.o)


# Clear default suffix rules
.SUFFIXES :
# Keep STAMPs and dependencies between calls
.PRECIOUS : %/.STAMP build/dep/%.d build/obj/%.o

all : test binaries

test : $(TEST_NAMES:%=build/test/%)

binaries : $(BIN_NAMES:%=build/bin/%)

build/bin/% : build/obj/%.o build/dep/%.d $(OBJECTS) build/bin/.STAMP makefile
	$(CXX) $(LDFLAGS) $< $(OBJECTS) -o $@

build/test/% : build/obj/%.o build/dep/%.d $(OBJECTS) build/test/.STAMP makefile
	$(CXX) $(LDFLAGS) $< $(OBJECTS) -o $@

build/obj/%.o : %.cpp build/dep/%.d build/obj/.STAMP
	$(CXX) $(CXXFLAGS) -c $< -o $@

include $(BIN_NAMES:%=build/dep/%.d)
include $(TEST_NAMES:%=build/dep/%.d)

build/dep/%.d : %.cpp build/dep/.STAMP
	@$(CXX) $(CXXFLAGS) -MM -MT '$@' $< > $@

%/.STAMP :
	@mkdir -p $(@D)
	@touch $@

clean :
	@rm -rf build/test build/bin build/dep build/obj

