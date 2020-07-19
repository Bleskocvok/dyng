#!/bin/bash

#   Copyright 2020 František Bráblík
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# creates a series of screenshots for given valid arguments to 'bin/generate'
# usage: $ ./make_screenshots.sh example_simple
# or: $ ./make_screenshots.sh gen_grid 8


function one_batch() {
    echo "$2 | name: '$1'"
    mkdir "$1" 2> /dev/null
    cd "$1"
    ../../bin/generate $3 | ../../bin/parallel_layout 4 "$2" 1024 640 | ../../bin/draw_states 1280 720
    for i in *.bmp; do
        convert $i `echo $i | cut -d'.' -f1`.png
    done
    rm *.bmp
    cd ..
}


mkdir "screenshots" 2> /dev/null
cd screenshots
params="$@"
one_batch "${params}_none" 0.00 "${params}"
one_batch "${params}_low" 0.04 "${params}"
one_batch "${params}_medium" 0.08 "${params}"
one_batch "${params}_high" 0.14 "${params}"
one_batch "${params}_ultra" 0.20 "${params}"
one_batch "${params}_full" 1.00 "${params}"

cd ..

