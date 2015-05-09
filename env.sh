#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]
then
	echo "Must be sourced!"
	exit 1
fi

moocov_root=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

export MOOCOV_ROOT="$moocov_root"
export MOOCOV_RUNTIME_ROOT="$moocov_root/runtime"
