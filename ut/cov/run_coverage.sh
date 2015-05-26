#! /bin/sh

cd ..
make
./cache_ut
mv *.gcda cov
cd cov
lcov --capture --directory ../  -b ../ --output-file cache.info
genhtml cache.info --output-directory ./
rm *.gcno *.gcda
