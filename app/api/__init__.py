#backend\app\api\__init__.py
from flask import Blueprint
from .endpoints import dwg, svg, layer

api_blueprint = Blueprint('api', __name__)

dwg.upload_dwg(api_blueprint)
dwg.get_dwg(api_blueprint)
svg.get_svg(api_blueprint)
layer.get_layers(api_blueprint)