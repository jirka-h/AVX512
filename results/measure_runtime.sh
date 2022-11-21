#!/bin/bash

#parse_log $file $name
#Looks for Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): in $file and stores the value to results.txt in format
#"${name} \t\t\t${runtime}
parse_log() {
  runtime=$(grep -Poh "Elapsed \(wall clock\) time \(h:mm:ss or m:ss\):\s+\K[0-9.:]+" "$1")
  if [ -n "$runtime" ]; then
    echo -e "${2} \t\t\t${runtime}" | tee --append results.txt
  else
    echo "WARN: '${1}' - could not parse 'Elapsed (wall clock) time' value"
  fi
}

cpu_name=$(lscpu | grep -Poh "^\s*Model name:\s+\K.*$" | sed -e 's/(.*)//g' | tr -cd 'A-Za-z0-9 ._-' | tr ' ' '_' | tr -s '_')
[ -d "$cpu_name" ] && rm -rf "./${cpu_name}"
mkdir "./$cpu_name"
pushd "./$cpu_name" || exit 1
echo -e "Test \t\t\tWall clock (m:ss)" > results.txt

while IFS= read -r -d '' bin; do
  #echo "$bin"
  [ -x "$bin" ] || { echo "'$bin' skipped - not executable"; continue; }
  file -i "$bin" | grep --quiet 'x-executable; charset=binary' || { echo "'$bin' skipped - failed file test"; continue; }

  bin_name=$(basename "$bin")
  if [[ "$bin_name" == "harmonic_series" ]]; then
    declare -A methods=([0]=HarmonicSeriesAVX512 [1]=HarmonicSeriesAVX256 [2]=HarmonicSeriesPlain [3]=HarmonicSeriesAVX512_four_vectors)
    for method in "${!methods[@]}"; do
      name=${methods[$method]}
      command=("turbostat" "-o" "${name}.turbostat" "/usr/bin/time" "-v" "perf" "stat" "${bin}" "$method" "4e9")
      echo "${command[*]}"
      sleep 5
      if "${command[@]}"  > "${name}.log" 2>&1; then
        parse_log "${name}.log" "$name"
      else
        echo "ERROR: ${bin_name} ${name} returned non-zero exit status"
      fi
    done
    continue
  fi

  command=("turbostat" "-o" "${bin_name}.turbostat" "/usr/bin/time" "-v" "perf" "stat" "${bin}")
  echo "${command[*]}"
  sleep 5
  if "${command[@]}" > "${bin_name}.log" 2>&1 ; then
    parse_log "${bin_name}.log" "${bin_name}"
  else
    echo "ERROR: ${bin_name} returned non-zero exit status"
  fi
done < <(find ../../bin -maxdepth 1 -type f -executable -print0 | sort -z)

echo "=================================================================================="
tail -v -n +1 results.txt
echo "=================================================================================="

