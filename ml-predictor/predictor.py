# predictor.py
import numpy as np
import joblib
import onnxruntime as ort

# Load sklearn model
model = joblib.load("burst_model.pkl")

# Example features: [arrival, priority, io_wait, prev_burst]
sample = np.array([[100, 2, 15, 7]])
sk_pred = model.predict(sample)
print("Sklearn prediction:", sk_pred[0])

# Load ONNX model
session = ort.InferenceSession("burst_model.onnx", providers=["CPUExecutionProvider"])
input_name = session.get_inputs()[0].name
output_name = session.get_outputs()[0].name
onnx_pred = session.run([output_name], {input_name: sample.astype(np.float32)})[0]
print("ONNX prediction:", onnx_pred[0][0])
