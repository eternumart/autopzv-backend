#backend/app/services/svg_service.py
from app.models.dwg import Layer  # Import Layer

def get_svg(dwg_id, layer_id):
    layer = Layer.query.filter_by(dwg_id=dwg_id, id=layer_id).first()

    if layer is not None:
        return layer.file_data

    return {"error": "No SVG data found for the given DWG ID and layer ID."}, 404
