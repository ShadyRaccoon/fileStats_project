#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Numar incorect de argumente!"
  exit 1
fi

char="$1"

corecte=0

while IFS= read -r line; do
    if [[ $line =~ ^[A-Z] ]] && [[ $line =~ [.?!]$ ]]; then
        if ! [[ $line =~ ",si" ]] && ! [[ $line =~ ", si" ]]; then
            if [[ $line == *"$char"* ]]; then
                ((corecte++))
            fi
        fi
    fi
done

echo "$corecte propozitii corecte numarate."
