FFDIR := /home/spm1501/fastflow
CC := icc
CFLAGS := -O3 -std=c++11 -pthread -I src/shared
REPORT = -qopt-report -qopt-report-phase=vec -qopt-report -qopt-report-phase=par -qopt-report-file=bin/$@.optrpt
MICFLAGS := -mmic -D_MIC
FF = -I $(FFDIR) -DNO_DEFAULT_MAPPING
.PHONY: \
	build build_xeon build_mic bin\
	test_mic test_xeon test_i5


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

test_xeon : build_xeon
	run/test.sh xeon

	
build_mic : baseline_mic components_mic sequential_mic

baseline_mic : src/baseline/baseline.cpp src/baseline/barrier.cpp src/shared/shared.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(MICFLAGS) -pthread $(subst bin,, $^) -o bin/$@

components_mic : src/shared/shared.cpp src/components/components.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(FF) $(MICFLAGS) $(subst bin,, $^) -o bin/$@

sequential_mic : src/shared/shared.cpp src/sequential/sequential.cpp bin
	$(CC) $(CFLAGS) $(REPORT) $(MICFLAGS) $(subst bin,, $^) -o bin/$@
	
test_mic : build_mic
	run/test.sh mic
	
	
test_i5 : build_xeon
	run/test.sh i5

	
clean_results :
	rm -rf tests DONE

clean_binaries :
	rm -rf bin

clean : clean_binaries clean_results

