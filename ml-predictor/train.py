# train.py
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
import joblib
import skl2onnx
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType


# load dataset (CSV) or generate synthetic
# X: features, y: next_cpu_burst


model = RandomForestRegressor(n_estimators=200)
model.fit(X, y)
# save sklearn model
joblib.dump(model, 'rf_model.joblib')
# export to ONNX
initial_type = [('float_input', FloatTensorType([None, X.shape[1]]))]
onx = convert_sklearn(model, initial_types=initial_type)
with open('model.onnx', 'wb') as f:
f.write(onx.SerializeToString())