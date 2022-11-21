#!/bin/bash

declare -A methods=([0]=HarmonicSeriesAVX512 [1]=HarmonicSeriesAVX256 [2]=HarmonicSeriesPlain [3]=HarmonicSeriesAVX512_four_vectors)

cpu_name=$(lscpu | grep -Poh "^\s*Model name:\s+\K.*$" | sed -e 's/(.*)//g' | tr -cd 'A-Za-z0-9 ._-' | tr ' ' '_' | tr -s '_')
dir_name=${cpu_name}_harmonic_series
[ -d "$dir_name" ] && rm -rf "./${dir_name}"
mkdir "./$dir_name"
pushd "./$dir_name" || exit 1

dir="$(pwd)/.."
for thread in 1 2 4 8 16; do
  mkdir "thread_$thread"
  pushd "thread_$thread" || exit 1

  for method in "${!methods[@]}"; do
    sleep 5
    name=${methods[$method]}
    sensors > "$name.sensors"
    options=()
    #options+=(--dry)
    seq $thread | turbostat -o "${name}.turbostat" parallel "${options[@]}" "/usr/bin/time ${dir}/harmonic_series $method 4e9 2>&1 | tee ${name}_{}.log"
    sensors >> "$name.sensors"
    echo
  done

  grep -H "power1:\|Sensor 1:" ./*sensors
  grep -H CPU ./*log
  grep -H -E -B1 "^-" ./*turbostat
  popd || exit 1
done



