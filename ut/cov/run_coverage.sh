#! /bin/sh

cd ..
make
./cache_ut
mv *.gcda cov
cd cov
lcov --capture --directory ../  -b ../ --output-file cache.info
if [ $? -ne 0 ] 
then 
echo "LCOV error, Maybe need install LCOV firstly"
exit -1
fi

genhtml cache.info --output-directory ./
rm *.gcno *.gcda
