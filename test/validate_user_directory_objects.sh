#!/bin/bash
# Usage: ./validate_user_directory_objects.sh path/to/user/directory

if [ -z "$1" ]
then
    echo "Usage: ./validate_user_directory_objects.sh path/to/user/directory"
    exit 1
fi

echo "Validating objects..."
ajv -s ../preference_backend_schema_2.json -d "$1/backend.json"  | grep -v ' valid$'
ajv -s ../preferences_schema_2.json -d "$1/config/preferences.json"  | grep -v ' valid$'
ajv -s ../layout_schema_2.json -d "$1/config/layouts/*.json"  | grep -v ' valid$'
ajv -s ../snippet_schema_1.json -d "$1/config/snippets/*.json"  | grep -v ' valid$'
ajv -s ../workspace_schema_1.json -d "$1/config/workspaces/*.json"  | grep -v ' valid$'
