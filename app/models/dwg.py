# backend/app/models/dwg.py
from app import db
from sqlalchemy.sql import func

class Dwg(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    filename = db.Column(db.String(120), nullable=False)
    layers = db.relationship('Layer', backref='dwg', lazy=True)
    created_at = db.Column(db.DateTime(timezone=True), default=func.now())
    updated_at = db.Column(db.DateTime(timezone=True), onupdate=func.now())

class Layer(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    layer_name = db.Column(db.String(120), nullable=False)  # New field
    file_data = db.Column(db.LargeBinary)
    dwg_id = db.Column(db.Integer, db.ForeignKey('dwg.id'), nullable=False)
    created_at = db.Column(db.DateTime(timezone=True), default=func.now())
    updated_at = db.Column(db.DateTime(timezone=True), onupdate=func.now())
