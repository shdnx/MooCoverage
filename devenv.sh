#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]
then
	echo "Must be sourced!"
	exit 1
fi

moocov_root=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# for some reason, linking fails with clang
export CC=gcc
export CXX=g++

# TODO: don't hardcode the "build" directory name
export PATH="$moocov_root/build/bin:$moocov_root/tools/testing:$PATH"

alias bmoocov='ninja -C "$moocov_root/build"'

source "$moocov_root/env.sh"
