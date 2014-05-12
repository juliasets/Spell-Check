#!/bin/bash

echo "Username:" ${1?"Usage: $0 USERNAME"}
user="$1"

cssh $user@ice{01..12}.ee.cooper.edu:31415



