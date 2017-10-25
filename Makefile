CC := g++
CFLAGS := -O3 -std=c++11 -I src/shared
REPORT := -qopt-report -qopt-report-phase=vec -qopt-report -qopt-report-phase=par
MICFLAGS := -mmic
FFDIR := /home/spm1501/fastflow
FF = -I $(FFDIR) -DNO_DEFAULT_MAPPING
.PHONY: \
	build build_xeon build_mic bin\
	testmic testxeon testi5


build : build_xeon build_mic


bin : 
	mkdir -p bin


build_xeon : baseline_xeon blocks_xeon components_xeon

baseline_xeon : src/shared/shared.cpp src/baseline/barrier.cpp src/baseline/baseline.cpp bin
	$(CC) $(CFLAGS) -pthread $(subst bin,, $^) -o bin/$@

blocks_xeon:

components_xeon:

testxeon : baseline_xeon components_xeon blocks_xeon
	run/test.sh xeon

	
build_mic : baseline_mic blocks_mic components_mic

baseline_mic : src/baseline/baseline.cpp src/baseline/barrier.cpp src/shared/shared.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(MICFLAGS) -pthread $(subst bin,, $^) -o bin/$@

blocks_mic :

components_mic :
	
testmic : baseline_mic components_mic blocks_mic
	run/test.sh mic
	
	
testi5 : baseline_xeon components_xeon blocks_xeon
	run/test.sh i5

	
cleanresults :
	rm -rf tests DONE

cleanbinaries :
	rm -rf bin

clean : cleanbinaries cleanresults

