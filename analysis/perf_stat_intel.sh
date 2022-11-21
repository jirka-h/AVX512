#!/bin/bash
perf_command=("perf" "stat" "-d" "-r" "3" "-M" "pipeline" "-e" "arith.divider_active" "--")
taskset_command=("taskset" "-c" "1" "/usr/bin/time" "-v")

cpu_name=$(lscpu | grep -Poh "^\s*Model name:\s+\K.*$" | sed -e 's/(.*)//g' | tr -cd 'A-Za-z0-9 ._-' | tr ' ' '_' | tr -s '_')
[ -d "$cpu_name" ] && rm -rf "./${cpu_name}"
mkdir "./$cpu_name"
pushd "./$cpu_name" || exit 1

while IFS= read -r -d '' bin; do
  #echo "$bin"
  [ -x "$bin" ] || { echo "'$bin' skipped - not executable"; continue; }
  file -i "$bin" | grep --quiet 'x-executable; charset=binary' || { echo "'$bin' skipped - failed file test"; continue; }

  bin_name=$(basename "$bin")
  if [[ "$bin_name" == "harmonic_series" ]]; then
    declare -A methods=([0]=HarmonicSeriesAVX512 [1]=HarmonicSeriesAVX256 [2]=HarmonicSeriesPlain [3]=HarmonicSeriesAVX512_four_vectors)
    for method in "${!methods[@]}"; do
      name=${methods[$method]}
      sleep 5
      echo "${perf_command[*]} ${taskset_command[*]} ${bin} $method 4e9"
      if "${perf_command[@]}" "${taskset_command[@]}" "${bin}" "$method" 4e9  > "${name}.log" 2>&1; then
        echo "Success for ${bin} $method 4e9 - see ${cpu_name}/${name}.log"
      else
        echo "ERROR: ${bin_name} ${name} returned non-zero exit status"
      fi
    done
    continue
  fi

  sleep 5
  echo "${perf_command[*]} ${taskset_command[*]} ${bin}"
  if "${perf_command[@]}" "${taskset_command[@]}" "${bin}" > "${bin_name}.log" 2>&1 ; then
    echo "Success for ${bin} - see ${cpu_name}/${bin_name}.log"
  else
    echo "ERROR: ${bin_name} returned non-zero exit status"
  fi
done < <(find ../../bin -maxdepth 1 -type f -executable -print0 | sort -z)
