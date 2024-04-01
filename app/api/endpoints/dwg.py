#backend/app/api/endpoints/dwg.py
import os
from flask import current_app as app
from flask import request, jsonify
from werkzeug.utils import secure_filename
from app.services.dwg_service import save_dwg
import tempfile


def allowed_file(filename):
    ALLOWED_EXTENSIONS = {'dwg'}
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def upload_dwg(api_blueprint):
    @api_blueprint.route('/api/upload-dwg', methods=['POST'])
    def upload():
        if 'file' not in request.files:
            return {"error": "No file part in the request."}, 400
        file = request.files['file']
        if file.filename == '':
            return {"error": "No file selected for uploading."}, 400
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            temp_file = tempfile.NamedTemporaryFile(delete=False)
            file.save(temp_file.name)
            print(f"Saved file to temporary location: {filename} and {temp_file.name}")
            file_id = save_dwg(filename, temp_file.name)
            print(f"Finished processing file: {filename} and {temp_file.name}")
            os.unlink(temp_file.name) 
            return {"message": "File successfully uploaded", "id": file_id}, 200
        else:
            return {"error": "Allowed file types are dwg."}, 400


