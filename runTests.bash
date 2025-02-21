#! /bin/bash
cd src/

make
cd ..

for f in "traces/fp_1.bz2" "traces/fp_2.bz2" "traces/int_1.bz2" "traces/int_2.bz2" "traces/mm_1.bz2" "traces/mm_2.bz2" 
do
	if [ -f "$f" ] 
	then
		echo "Testing file: $f"
		bunzip2 -kc $f | ./src/predictor $1
	else
		echo ""
	fi
done