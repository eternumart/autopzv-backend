#backend/app/api/endpoints/dwg.py
import os
from flask import current_app as app
from flask import request, jsonify
from werkzeug.utils import secure_filename
from app.services.dwg_service import save_dwg
import tempfile
from app.models.dwg import Dwg
from flasgger import swag_from


def get_dwg(api_blueprint):
    @api_blueprint.route('/api/get-dwg/<dwg_id>', methods=['GET'])
    @swag_from({
        'summary': 'Получить детали DWG',
        'description': 'Возвращает минимальные и максимальные координаты для указанного DWG ID.',
        'responses': {
            200: {
                'description': 'Детали DWG',
                'schema': {
                    'type': 'object',
                    'properties': {
                        'min_x': {'type': 'number'},
                        'min_y': {'type': 'number'},
                        'max_x': {'type': 'number'},
                        'max_y': {'type': 'number'}
                    }
                }
            },
            404: {
                'description': 'DWG с указанным ID не найден'
            }
        }
    })
    def get_dwg(dwg_id):
        dwg = Dwg.query.get(dwg_id)
        if dwg is not None:
            return {"min_x": dwg.min_x, "min_y": dwg.min_y, "max_x": dwg.max_x, "max_y": dwg.max_y}
        else:
            return {"error": "DWG с указанным ID не найден."}, 404


def allowed_file(filename):
    ALLOWED_EXTENSIONS = {'dwg'}
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def upload_dwg(api_blueprint):
    @api_blueprint.route('/api/upload-dwg', methods=['POST'])
    @swag_from({
        'summary': 'Загрузить DWG файл',
        'description': 'Загружает DWG файл и сохраняет его в базе данных.',
        'consumes': ['multipart/form-data'],
        'parameters': [
            {
                'name': 'file',
                'in': 'formData',
                'type': 'file',
                'required': True,
                'description': 'DWG файл для загрузки'
            }
        ],
        'responses': {
            200: {
                'description': 'Файл успешно загружен',
                'schema': {
                    'type': 'object',
                    'properties': {
                        'message': {'type': 'string'},
                        'id': {'type': 'integer'}
                    }
                }
            },
            400: {
                'description': 'Сообщение об ошибке'
            }
        }
    })
    def upload():
        if 'file' not in request.files:
            return {"error": "Файл не найден в запросе."}, 400
        file = request.files['file']
        if file.filename == '':
            return {"error": "Файл не выбран для загрузки."}, 400
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            temp_file = tempfile.NamedTemporaryFile(delete=False)
            file.save(temp_file.name)
            print(f"Saved file to temporary location: {filename} and {temp_file.name}")
            file_id = save_dwg(filename, temp_file.name)
            print(f"Finished processing file: {filename} and {temp_file.name}")
            os.unlink(temp_file.name) 
            return {"message": "Файл успешно загружен", "id": file_id}, 200
        else:
            return {"error": "Разрешены только файлы типа dwg."}, 400


