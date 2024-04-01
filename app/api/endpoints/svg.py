# backend/app/api/endpoints/svg.py
from flask import send_file
from app.services.svg_service import get_svg as get_svg_service
import io

def get_svg(api_blueprint):
    @api_blueprint.route('/api/get-svg/<dwg_id>/<layer_id>', methods=['GET'])
    def get_svg(dwg_id, layer_id):
        svg_data = get_svg_service(dwg_id, layer_id)
        if isinstance(svg_data, tuple) and 'error' in svg_data[0]:
            return svg_data
        svg_file = io.BytesIO(svg_data)
        return send_file(svg_file, mimetype='image/svg+xml')

