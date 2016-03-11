#!/bin/bash
minelemsize=100000
maxelemsize=100000000
minthreadcount=1
maxthreadcount=8
types=(float int)
generators=(zero random)
iterations=10
rm run.log
touch run.log

elemsize=$minelemsize
while [  $elemsize -le $maxelemsize ]; do
	echo Elemsize is $elemsize

	threadcount=$minthreadcount
	while [ $threadcount -le $maxthreadcount ]; do
		echo Threadcount is $threadcount
		
		for type in "${types[@]}"; do
			#echo Type is $type
			for generator in "${generators[@]}"; do
				#echo Generator is $generator
				./build/tbb-sample-sort --generator $generator --type $type  --num_threads $threadcount --num_elements $elemsize --iterations $iterations >> run.log
			done
		done

		let threadcount=threadcount*2
	done

        let elemsize=elemsize*10
done

