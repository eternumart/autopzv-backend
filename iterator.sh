#!/bin/bash

# DWG ID
dwg_id=36

# Define column widths
layer_id_width=10
colors_width=30
stroke_widths_width=25
dash_types_width=20

# Print the table header
printf "%-${layer_id_width}s %-${colors_width}s %-${stroke_widths_width}s %-${dash_types_width}s\n" "Layer ID" "Unique Colors" "Unique Stroke Widths" "Unique Dash Types"

# Get the list of layers for the DWG
layers=$(curl -s -X GET http://192.168.0.20:5000/api/get-layers/$dwg_id | jq -r '.[].id')

# Function to convert hex color to 24-bit terminal color code
hex_to_24bit() {
    printf "\033[38;2;%d;%d;%dm%s\033[0m" $((0x${1:1:2})) $((0x${1:3:2})) $((0x${1:5:2})) $1
}

# Iterate over each layer
for layer_id in $layers; do
    # Get the SVG data for the layer
    svg_data=$(curl -s -X GET http://192.168.0.20:5000/api/get-svg/$dwg_id/$layer_id)

    # Extract the unique colors, stroke widths, and dash types from the SVG data
    colors=$(echo "$svg_data" | grep -oP '(?<=fill=")[^"]*' | sort | uniq)
    stroke_widths=$(echo "$svg_data" | grep -oP '(?<=stroke-width:)[^p]*' | sort | uniq)
    dash_types=$(echo "$svg_data" | grep -oP '(?<=stroke-dasharray:)[^;]*' | sort | uniq)

    # If no colors were found, set colors to 'None'
    if [ -z "$colors" ]; then
        colors="None"
    fi

    # If no stroke widths were found, set stroke widths to 'None'
    if [ -z "$stroke_widths" ]; then
        stroke_widths="None"
    fi

    # If no dash types were found, set dash types to 'None'
    if [ -z "$dash_types" ]; then
        dash_types="None"
    fi

    # Print the layer ID
    printf "%-${layer_id_width}s " "$layer_id"

    # Print each color
    color_str=""
    for color in $colors; do
        if [[ $color == "#"* ]]; then
            color_str+=$(hex_to_24bit $color)", "
        else
            color_str+="$color, "
        fi
    done
    printf "%-${colors_width}s " "$color_str"

    # Print each stroke width
    stroke_width_str=""
    for stroke_width in $stroke_widths; do
        stroke_width_str+="$stroke_width, "
    done
    printf "%-${stroke_widths_width}s " "$stroke_width_str"

    # Print each dash type
    dash_type_str=""
    for dash_type in $dash_types; do
        dash_type_str+="$dash_type, "
    done
    printf "%-${dash_types_width}s " "$dash_type_str"

    # Print a newline at the end of each layer's colors, stroke widths, and dash types
    printf "\n"
done
