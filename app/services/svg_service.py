#backend/app/services/svg_service.py
from app.models.dwg import Layer, Dwg  # Import Layer
import re

def get_svg(dwg_id, layer_id, zoom=None, grid_x=None, grid_y=None):
    layer = Layer.query.filter_by(dwg_id=dwg_id, id=layer_id).first()
    dwg = Dwg.query.get(dwg_id)

    if layer is not None:
        svg_data = layer.file_data.decode('utf-8')

        if zoom is not None and grid_x is not None and grid_y is not None:
            grid_size = int(zoom) if zoom is not None else None
            grid_width = (dwg.max_x - dwg.min_x) / grid_size
            grid_height = (dwg.max_y - dwg.min_y) / grid_size
            min_x = dwg.min_x + grid_width * int(grid_x)
            min_y = dwg.min_y + grid_height * int(grid_y)
            max_x = min_x + grid_width
            max_y = min_y + grid_height
            svg_data = re.sub(r'viewBox="[^"]*"', f'viewBox="{min_x} {min_y} {max_x - min_x} {max_y - min_y}"', svg_data)

        return svg_data.encode('utf-8')

    return {"error": "No SVG data found for the given DWG ID and layer ID."}, 404
