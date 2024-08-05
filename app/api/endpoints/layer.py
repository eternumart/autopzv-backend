# backend/app/api/endpoints/layer.py
from flask import jsonify
from app.services.layer_service import get_layer_names
from flasgger import swag_from

def get_layers(api_blueprint):
    @api_blueprint.route('/api/get-layers/<dwg_id>', methods=['GET'])
    @swag_from({
        'summary': 'Получить имена слоев',
        'description': 'Возвращает список имен слоев для указанного DWG ID.',
        'parameters': [
            {
                'name': 'dwg_id',
                'in': 'path',
                'type': 'string',
                'required': True,
                'description': 'ID DWG файла'
            }
        ],
        'responses': {
            200: {
                'description': 'Список имен слоев',
                'schema': {
                    'type': 'array',
                    'items': {
                        'type': 'object',
                        'properties': {
                            'id': {'type': 'integer'},
                            'name': {'type': 'string'}
                        }
                    }
                }
            },
            404: {
                'description': 'DWG с указанным ID не найден'
            }
        }
    })
    def get_layers(dwg_id):
        layer_names = get_layer_names(dwg_id)
        return jsonify(layer_names)
