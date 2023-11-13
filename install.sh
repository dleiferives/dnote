#!/bin/bash

# Path to the new executable
new_executable_path="./dnote"
# Path to the config file
config_file_path="./dnote.config"

# Destination directory
# Use '/usr/local/bin' for a system-wide installation (might require sudo)
# or '$HOME/bin' for a user-specific installation
destination="$HOME/bin"
destination_config_path="$HOME/.config/dnote/"

# Destination executable and config file paths
destination_executable="$destination/dnote"
destination_config="$destination_config_path/dnote.config"

# Check if destination directory exists, if not create it
if [ ! -d "$destination_config_path" ]; then
    mkdir -p "$destination_config_path"
fi

# Check if destination directory exists, if not create it
if [ ! -d "$destination" ]; then
    mkdir -p "$destination"
fi

# Add execute permissions to the new executable
chmod +x "$new_executable_path"

# Check if the executable already exists in the destination
if [ -f "$destination_executable" ]; then
    echo "Updating dnote..."
    rm -f "$destination_executable"
else
    echo "Installing dnote..."
fi

# Move the new executable to the destination
cp "$new_executable_path" "$destination_executable"

# Check if the config file exists and copy it
if [ -f "$config_file_path" ]; then
    echo "Copying configuration file..."
    cp "$config_file_path" "$destination_config"
else
    echo "Configuration file not found. Skipping config copy."
fi

echo "dnote has been successfully installed/updated."

