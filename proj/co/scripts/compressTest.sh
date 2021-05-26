#!/bin/bash

start_time() {
	START=$(date +%s.%N)
}

disp_diff_time() {
	END=$(date +%s.%N)
	DIFF=`echo "scale=2; (($END - $START) * 100) / 100" | bc`
	echo "time: " $DIFF"s"
}

calc_compress_ratio() {
	size=$(ls -l $1 | awk -e '/ / {print $5}')
	size_ratio=`echo "scale=2; ($size *100/ $in_size)" | bc -l`
	echo "Compress ratio: "$size_ratio"%"
}

print_size() {
	size=$(du -h $1 | awk -e '/^[0-9]/ {print $1}')
	echo "size: " $size
}


arg=( "$@" )
in_file=${arg[0]}
in_size=$(ls -l $in_file | awk -e '/ / {print $5}')
out_path="test/"

compress_ext=("7z" "tar" "tar.gz" "tar.bz2" "tar.xz" "bz2" "gz" "lzma" "xz" "rar")
compress_cmd=("7z a  $out_path$in_file.${compress_ext[0]} $in_file > /dev/null" \
			"tar cf  $out_path$in_file.${compress_ext[1]} $in_file > /dev/null" \
			"tar cvzf  $out_path$in_file.${compress_ext[2]} $in_file > /dev/null" \
			"tar cvfj  $out_path$in_file.${compress_ext[3]} $in_file > /dev/null" \
			"tar cvfJ  $out_path$in_file.${compress_ext[4]} $in_file > /dev/null" \
			"bzip2 -c $in_file > $out_path$in_file.${compress_ext[5]}" \
			"gzip -c $in_file > $out_path$in_file.${compress_ext[6]}" \
			"lzma -c $in_file > $out_path$in_file.${compress_ext[7]}" \
			"xz -c $in_file > $out_path$in_file.${compress_ext[8]}" \
			"rar a  $out_path$in_file.${compress_ext[9]} $in_file > /dev/null")

compress() {
	echo "Compress " $1
	cmd=`echo $@ | awk '{$1 = ""; print $0}'`
	start_time
	eval $cmd
	disp_diff_time
	print_size $out_path$in_file.$1
	calc_compress_ratio $out_path$in_file.$1
	echo " "
}

function main() {
	
	mkdir -p test
	echo "Input file: " $in_file
	print_size $in_file
	echo " "
	
	i=0
	for idx in ${!compress_cmd[@]};
	do
		compress ${compress_ext[$i]} ${compress_cmd[$idx]}
		i=$((i+1))
	done	
}	
main "$@"
