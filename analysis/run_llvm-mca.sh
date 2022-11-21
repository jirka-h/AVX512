#!/bin/bash
command -v llvm-mca >/dev/null 2>&1 || { echo >&2 "llvm-mca is not installed.  Aborting."; exit 1; }


cpu_name=$(lscpu | grep -Poh "^\s*Model name:\s+\K.*$" | sed -e 's/(.*)//g' | tr -cd 'A-Za-z0-9 ._-' | tr ' ' '_' | tr -s '_')
dir_name=${cpu_name}_llvm-mca
[ -d "$dir_name" ] && rm -rf "./${dir_name}"
mkdir "./$dir_name"
pushd "./$dir_name" || exit 1

while IFS= read -r -d '' file; do
  basename=$(basename -s .c "$file")
  #echo "${file} $basename"
  if gcc -O3 -mavx512f -mfma -S "$file"  -o - | llvm-mca -iterations 10000 -timeline -bottleneck-analysis -retire-stats > "${basename}".log 2>&1; then
    echo "Success for ${file} - see '${dir_name}/${basename}.log' for results"
  else
    echo "ERROR: llvm-mca has failed for ${file}"
  fi
done < <(find ../ -maxdepth 1 -name "*.c" -print0 | sort -z)
