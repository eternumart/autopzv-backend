#backend/app/__init__.py
from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flasgger import Swagger
from dotenv import load_dotenv
from urllib.parse import quote_plus
import os

load_dotenv()
print("DB_HOST:", os.getenv('DB_HOST'))
print("DB_PORT:", os.getenv('DB_PORT'))
print("DB_NAME:", os.getenv('DB_NAME'))
print("DB_USER:", os.getenv('DB_USER'))

password = quote_plus(os.getenv('DB_PASSWORD'))
print("DB_PASSWORD:", password)

app = Flask(__name__)

swagger = Swagger(app, config={
    "headers": [],
    "specs": [
        {
            "endpoint": 'api/docs',
            "route": '/api/docs.json',
        }
    ],
    "static_url_path": "/flasgger_static",
    "swagger_ui": True,
    "specs_route": "/api/docs"
})

app.config['SQLALCHEMY_DATABASE_URI'] = f"postgresql://{os.getenv('DB_USER')}:{password}@{os.getenv('DB_HOST')}:{os.getenv('DB_PORT')}/{os.getenv('DB_NAME')}"
db = SQLAlchemy(app)

def create_app():
    from .api import api_blueprint
    app.register_blueprint(api_blueprint)

    with app.app_context():
        db.create_all()

    return app
