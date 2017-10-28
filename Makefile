CC := icc
CFLAGS := -O3 -std=c++11 -pthread -I src/shared
REPORT := -qopt-report -qopt-report-phase=vec -qopt-report -qopt-report-phase=par
MICFLAGS := -mmic -D_MIC
FFDIR := /home/spm1501/fastflow
FF = -I $(FFDIR) -DNO_DEFAULT_MAPPING
.PHONY: \
	build build_xeon build_mic bin\
	testmic testxeon testi5


build : build_xeon build_mic


bin : 
	mkdir -p bin


build_xeon : baseline_xeon components_xeon sequential_xeon

baseline_xeon : src/shared/shared.cpp src/baseline/barrier.cpp src/baseline/baseline.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(subst bin,, $^) -o bin/$@

components_xeon : src/shared/shared.cpp src/components/components.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(FF) $(subst bin,, $^) -o bin/$@

sequential_xeon : src/shared/shared.cpp src/sequential/sequential.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(subst bin,, $^) -o bin/$@

testxeon : build_xeon
	run/test.sh xeon

	
build_mic : baseline_mic components_mic sequential_mic

baseline_mic : src/baseline/baseline.cpp src/baseline/barrier.cpp src/shared/shared.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(MICFLAGS) -pthread $(subst bin,, $^) -o bin/$@

components_mic : src/shared/shared.cpp src/components/components.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(FF) $(MICFLAGS) $(subst bin,, $^) -o bin/$@

sequential_mic : src/shared/shared.cpp src/sequential/sequential.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(MICFLAGS) $(subst bin,, $^) -o bin/$@
	
testmic : build_mic
	run/test.sh mic
	
	
testi5 : build_xeon
	run/test.sh i5

	
cleanresults :
	rm -rf tests DONE

cleanbinaries :
	rm -rf bin

clean : cleanbinaries cleanresults

