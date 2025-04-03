#!/bin/bash
# Usage: ./validate_mongodb_objects.sh [remote-host]

if [ ! -z "$1" ]
then
    echo "Exporting data from mongodb on $1..."
    ssh $1 /bin/bash << EOF
mongoexport --quiet -d CARTA -c preferences > preferences.json
mongoexport --quiet -d CARTA -c layouts -f layout > layouts.json
mongoexport --quiet -d CARTA -c snippets -f snippet > snippets.json
mongoexport --quiet -d CARTA -c workspaces -f workspace > workspaces.json
EOF

    echo "Downloading data..."
    scp $1:preferences.json $1:layouts.json $1:snippets.json $1:workspaces.json data/

    echo "Cleaning up..."
    ssh $1 rm preferences.json layouts.json snippets.json workspaces.json

    IFS=$'\n'
    set -f

    echo "Extracting preferences objects..."
    
    mkdir -p data/preferences

    for doc in $(cat < data/preferences.json)
    do
        filename=`echo $doc | jq -r '._id."$oid"'`.json
        echo $doc | jq -r 'del(._id,.username)' > "data/preferences/$filename"
    done

    for object in "layout" "snippet" "workspace"
    do
        echo "Extracting ${object} objects..."
        mkdir -p "data/${object}s"
        
        for doc in $(cat < "data/${object}s.json")
        do
            filename=`echo $doc | jq -r '._id."$oid"'`.json
            echo $doc | jq -r ".${object}" > "data/${object}s/$filename"
        done 
    done
fi

echo "Validating objects..."
ajv -s ../preferences_schema_2.json -d "data/preferences/*.json"  | grep -v ' valid$'
ajv -s ../layout_schema_2.json -d "data/layouts/*.json"  | grep -v ' valid$'
ajv -s ../snippet_schema_1.json -d "data/snippets/*.json"  | grep -v ' valid$'
ajv -s ../workspace_schema_1.json -d "data/workspaces/*.json"  | grep -v ' valid$'
