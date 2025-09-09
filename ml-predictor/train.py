import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error
import joblib
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType


# 1. Generate synthetic workload data
def generate_data(n=2000):
    np.random.seed(42)
    data = []
    for i in range(n):
        arrival = np.random.randint(0, 1000)          # arrival time
        priority = np.random.randint(0, 5)            # priority level (0 = high)
        io_wait = np.random.randint(0, 50)            # past I/O wait
        prev_burst = np.random.randint(1, 20)         # previous CPU burst
        burst = prev_burst + np.random.randint(-3, 5) + priority
        burst = max(1, burst)
        data.append([arrival, priority, io_wait, prev_burst, burst])
    df = pd.DataFrame(data, columns=["arrival", "priority", "io_wait", "prev_burst", "burst"])
    return df

# 2. Train model
df = generate_data()
X = df[["arrival", "priority", "io_wait", "prev_burst"]]
y = df["burst"]

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

model = RandomForestRegressor(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# 3. Evaluate
preds = model.predict(X_test)
mae = mean_absolute_error(y_test, preds)
print(f"Validation MAE: {mae:.2f}")

# 4. Save model (sklearn + ONNX)
joblib.dump(model, "burst_model.pkl")

initial_type = [("input", FloatTensorType([None, X.shape[1]]))]
onnx_model = convert_sklearn(model, initial_types=initial_type)
with open("burst_model.onnx", "wb") as f:
    f.write(onnx_model.SerializeToString())

print("✅ Model exported to burst_model.onnx")
