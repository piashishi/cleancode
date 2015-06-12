# cleancode
Libcache Library

1: compile source code
Support 32/64 Linux OS. 
cd src
make

2: run ut
cd ut
make

3:support debug/release version
release:
make
debug:
make ver=debug

4: support coverage (LCOV)
cd ut/cov
chmod +x run_coverage.sh
./run_coverage.sh
