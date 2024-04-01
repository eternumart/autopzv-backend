# backend/app/api/endpoints/layer.py
from flask import jsonify
from app.services.layer_service import get_layer_names

def get_layers(api_blueprint):
    @api_blueprint.route('/api/get-layers/<dwg_id>', methods=['GET'])
    def get_layers(dwg_id):
        layer_names = get_layer_names(dwg_id)
        return jsonify(layer_names)
