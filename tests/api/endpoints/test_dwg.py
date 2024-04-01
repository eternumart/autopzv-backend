#backend/tests/api/endpoints/test_dwg.py
import os
import json
import pytest

from app import create_app, db
from app.models.dwg import Dwg, Layer

@pytest.fixture
def client():
    app = create_app()
    app.config['TESTING'] = True

    with app.test_client() as client:
        with app.app_context():
            db.create_all()
        yield client

        with app.app_context():
            db.drop_all()

def test_upload_dwg(client):
    filename = "test.dwg"
    path = "/path/to/test.dwg"  # Replace with the actual path to a test DWG file
    data = {
        'file': (open(path, 'rb'), filename)
    }
    response = client.post('/api/upload-dwg', content_type='multipart/form-data', data=data)

    assert response.status_code == 200
    assert b"File successfully uploaded" in response.data

    # Parse the response data to get the ID of the uploaded file
    response_data = json.loads(response.data)
    file_id = response_data.get('id')

    # Check that a Dwg object was created
    dwg = Dwg.query.get(file_id)
    assert dwg is not None
    assert dwg.filename == filename

    # Check that Layer objects were created
    layers = Layer.query.filter_by(dwg_id=file_id).all()
    assert len(layers) > 0
