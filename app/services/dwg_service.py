#backend/app/services/dwg_service.py
from app import db
from app.models.dwg import Dwg, Layer  # Import Layer
import subprocess
import os

def save_dwg(filename, temp_filename):
    # Create a new Dwg for the DWG file
    new_dwg = Dwg(filename=filename)
    db.session.add(new_dwg)
    db.session.commit()
    os.system(f"ls -al {temp_filename}")
    print(f"Processing file: {filename} at temporary location: {temp_filename}")  # Print the filename and temporary filename
    print(f"New DWG ID: {new_dwg.id}")

    # Get the list of layers
    result = subprocess.run(["/home/be-admin/libredwg/examples/dwg2svg2v4", "-f", temp_filename], capture_output=True, text=True)
    #print(result.stdout)  # Print the output from dwg2svg2v2
    #print(result.stderr)  # Print the error messages from dwg2svg2v2
    if result.returncode != 0:
        print(f"dwg2svg2v4 failed: {result.stderr}")
        return

    layers = result.stdout.splitlines()

    #print(f"Number of layers: {len(layers)}")  # Print the number of layers

    for layer_name in layers:
        # Generate SVG for each layer
        result = subprocess.run(["/home/be-admin/libredwg/examples/dwg2svg2v4", "-f", temp_filename, "-l", layer_name], capture_output=True, text=True)
        #print(result.stdout)  # Print the output from dwg2svg2v2
        if result.returncode != 0:
            print(f"dwg2svg2v4 failed: {result.stderr}")
            continue

        # Split the command output into lines
        lines = result.stdout.splitlines()

        # Find the index of the first line that starts with '<', which is the start of the SVG data
        svg_start = next((i for i, line in enumerate(lines) if line.startswith('<')), None)

        # Add SVG header and footer
        svg_header = '<svg xmlns="http://www.w3.org/2000/svg">'
        svg_footer = '</svg>'
        svg_data = '\n'.join([svg_header] + lines[svg_start:] + [svg_footer])

        # Save the SVG data to the database as a new Layer
        new_layer = Layer(layer_name=layer_name, file_data=svg_data.encode('utf-8'), dwg_id=new_dwg.id)
        db.session.add(new_layer)
        db.session.commit()

    return new_dwg.id

    
    return new_dwg.id


