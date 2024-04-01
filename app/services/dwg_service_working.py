#backend/app/services/dwg_service_working.py
import ctypes
from app import db
from app.models.dwg import Dwg
import subprocess

def save_dwg(filename, temp_filename):
    result = subprocess.run(["/home/be-admin/libredwg/examples/dwg2svg2", temp_filename], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"dwg2svg2 failed: {result.stderr}")
        return

    new_dwg = Dwg(filename=filename, file_data=result.stdout.encode('utf-8'))
    db.session.add(new_dwg)
    db.session.commit()

    return new_dwg.id

def get_svg(id):
    dwg = Dwg.query.get(id)

    if dwg is not None:
        return dwg.file_data

    return None
