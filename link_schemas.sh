#!/usr/bin/env bash
cd "$(dirname "$0")"
printf "Linking schemas..."
cd ../node_modules
rm -f carta-schemas
ln -s ../schemas carta-schemas
