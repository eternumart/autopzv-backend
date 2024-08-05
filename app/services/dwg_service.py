#backend/app/services/dwg_service.py
from app import db
from app.models.dwg import Dwg, Layer
import subprocess
import os

def save_dwg(filename, temp_filename):
    new_dwg = Dwg(filename=filename)
    db.session.add(new_dwg)
    db.session.commit()
    os.system(f"ls -al {temp_filename}")
    print(f"Processing file: {filename} at temporary location: {temp_filename}")
    print(f"New DWG ID: {new_dwg.id}")

    result = subprocess.run(["/root/libredwg/examples/dwg2svg2v8", "-f", temp_filename], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"dwg2svg2v8 failed: {result.stderr}")
        return

    lines = result.stdout.splitlines()

    coords_line = lines[-1]
    xmin, ymin, xmax, ymax = map(float, coords_line.split(","))
    width = xmax - xmin
    height = ymax - ymin

    new_dwg.min_x = xmin
    new_dwg.min_y = ymin
    new_dwg.max_x = xmax
    new_dwg.max_y = ymax
    db.session.commit()

    layer_names = lines[:-1]

    for layer_name in layer_names:
        result = subprocess.run(["/root/libredwg/examples/dwg2svg2v8", "-f", temp_filename, "-l", layer_name], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"dwg2svg2v8 failed: {result.stderr}")
            continue
        lines = result.stdout.splitlines()
        svg_start = next((i for i, line in enumerate(lines) if line.startswith('<')), None)

        svg_header = f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="{xmin} {ymin} {width} {height}">'
        svg_footer = '</svg>'
        svg_data_lines = lines[svg_start:-1] 
        svg_data = '\n'.join([svg_header] + svg_data_lines + [svg_footer])

        new_layer = Layer(layer_name=layer_name, file_data=svg_data.encode('utf-8'), dwg_id=new_dwg.id)
        db.session.add(new_layer)
        db.session.commit()

    return new_dwg.id


