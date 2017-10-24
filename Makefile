CC := g++
CFLAGS := -O3 -std=c++11
MICFLAGS := 
DATE_DIR := "$(date +%F@%R)"
.PHONY: \
	build build_xeon build_mic bin\
	testmic testxeon testi5


build : build_xeon build_mic


bin : 
	mkdir -p bin


build_xeon : baseline_xeon blocks_xeon components_xeon

baseline_xeon : src/baseline/baseline.cpp src/baseline/barrier.cpp src/baseline/barrier.hpp bin
	$(CC) $(CFLAGS) -pthread $(subst bin,, $^) -o bin/$@

blocks_xeon:

components_xeon:

testxeon : baseline_xeon components_xeon blocks_xeon
	run/test.sh xeon

	
build_mic : baseline_mic blocks_mic components_mic

baseline_mic :

blocks_mic :

components_mic :
	
testmic : baseline_mic components_mic blocks_mic
	run/test.sh mic
	
	
testi5 : baseline_xeon components_xeon blocks_xeon
	run/test.sh i5

	
clean :
	rm -rf bin test

cleanresults :
	rm -rf run/20*
