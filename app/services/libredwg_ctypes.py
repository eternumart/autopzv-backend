#backend/app/services/libredwg_ctypes.py
import ctypes

libredwg = ctypes.CDLL("/home/be-admin/libredwg/examples/dwg2svg2.so")

class DwgStructure(ctypes.Structure):
    pass

# New bindings
libredwg.get_layers.argtypes = [ctypes.POINTER(DwgStructure), ctypes.POINTER(ctypes.c_int)]
libredwg.get_layers.restype = ctypes.POINTER(ctypes.c_char_p)

libredwg.output_SVG_for_layer.argtypes = [ctypes.POINTER(DwgStructure), ctypes.c_char_p]
libredwg.output_SVG_for_layer.restype = None
