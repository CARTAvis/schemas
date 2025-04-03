#!/bin/bash
# Usage: ./validate_mongodb_objects.sh [[remote-host] remote-host...]

for host in "$@"
do
    mkdir -p  "data/controller/${host}" "data/controller/${host}/config.d"
    
    echo "Downloading config files from ${host}..."
    scp "$host:/etc/carta/*.json" "data/controller/${host}"
    scp "$host:/etc/carta/config.d/*.json" "data/controller/${host}/config.d"
done

echo "Validating objects..."
ajv -s ../preference_backend_schema_2.json -d "data/controller/*/backend.json"  | grep -v ' valid$'
ajv -c ajv-formats -s ../controller_config_schema_2.json -d "data/controller/*/config.json"  | grep -v ' valid$'
ajv -c ajv-formats -s ../controller_config_schema_2.json -d "data/controller/*/config.d/*.json"  | grep -v ' valid$'
