# backend/app/api/endpoints/svg.py
from flask import send_file
from app.services.svg_service import get_svg as get_svg_service
import io
from flasgger import swag_from

def get_svg(api_blueprint):
    @api_blueprint.route('/api/get-svg/<dwg_id>/<layer_id>', methods=['GET'])
    @api_blueprint.route('/api/get-svg/<dwg_id>/<layer_id>/<grid_x>/<grid_y>/<zoom>', methods=['GET'])
    @swag_from({
        'summary': 'Получить SVG файл',
        'description': 'Возвращает SVG файл для указанного DWG ID и Layer ID. Опционально можно указать параметры сетки и масштаба.',
        'parameters': [
            {'name': 'dwg_id', 'in': 'path', 'type': 'string', 'required': True, 'description': 'ID DWG файла'},
            {'name': 'layer_id', 'in': 'path', 'type': 'string', 'required': True, 'description': 'ID слоя'},
            {'name': 'grid_x', 'in': 'path', 'type': 'string', 'required': False, 'description': 'Координата X сетки'},
            {'name': 'grid_y', 'in': 'path', 'type': 'string', 'required': False, 'description': 'Координата Y сетки'},
            {'name': 'zoom', 'in': 'path', 'type': 'string', 'required': False, 'description': 'Уровень масштаба'}
        ],
        'responses': {
            200: {
                'description': 'SVG файл',
                'content': {
                    'image/svg+xml': {}
                }
            },
            404: {
                'description': 'Сообщение об ошибке'
            }
        }
    })
    def get_svg(dwg_id, layer_id, grid_x=None, grid_y=None, zoom=None):
        svg_data = get_svg_service(dwg_id, layer_id, zoom, grid_x, grid_y)
        if isinstance(svg_data, tuple) and 'error' in svg_data[0]:
            return svg_data
        svg_file = io.BytesIO(svg_data)
        return send_file(svg_file, mimetype='image/svg+xml')

