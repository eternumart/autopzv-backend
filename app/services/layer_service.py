#backend/app/services/layer_service.py
from app import db
from app.models.dwg import Dwg, Layer  # Import Layer

def get_layer_names(dwg_id):
    if not str(dwg_id).isdigit():
        return []

    layers = Layer.query.filter_by(dwg_id=dwg_id).all()
    layer_info = [{"id": layer.id, "name": layer.layer_name} for layer in layers]
    return layer_info
