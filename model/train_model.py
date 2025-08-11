import os
from pathlib import Path
import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.preprocessing import LabelEncoder
from sklearn.model_selection import train_test_split

ROOT = Path(__file__).resolve().parents[1]
DATA_DIR = ROOT / "data" / "data"
MODEL_DIR = ROOT / "model"
MODEL_DIR.mkdir(parents=True, exist_ok=True)

CSV_AUG = DATA_DIR / "crack_depth_dataset_augmented.csv"
CSV_RAW = DATA_DIR / "crack_depth_dataset.csv"
CSV_PATH = CSV_AUG if CSV_AUG.exists() else CSV_RAW

df = pd.read_csv(CSV_PATH)

df.columns = [c.strip().lower() for c in df.columns]
if "depth_cm" not in df.columns:
    num_cols = [c for c in df.columns if pd.api.types.is_numeric_dtype(df[c])]
    if not num_cols:
        raise ValueError("No numeric column found for depth.")
    depth_col = num_cols[0]
else:
    depth_col = "depth_cm"

if "label" not in df.columns and "severity" in df.columns:
    df.rename(columns={"severity": "label"}, inplace=True)
elif "label" not in df.columns:
    non_num = [c for c in df.columns if not pd.api.types.is_numeric_dtype(df[c])]
    if not non_num:
        raise ValueError("No label column found.")
    df.rename(columns={non_num[0]: "label"}, inplace=True)

df["label"] = df["label"].astype(str).str.strip()

le = LabelEncoder()
df["label_encoded"] = le.fit_transform(df["label"])
classes = list(le.classes_)
(MODEL_DIR / "labels.txt").write_text("\n".join(classes), encoding="utf-8")

X = df[[depth_col]].values.astype(np.float32)
y = df["label_encoded"].values
x_train, x_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(1,)),
    tf.keras.layers.Dense(64, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax')
])
model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])

callbacks = [
    tf.keras.callbacks.EarlyStopping(monitor="val_loss", patience=5, restore_best_weights=True),
    tf.keras.callbacks.ReduceLROnPlateau(monitor="val_loss", factor=0.5, patience=3, min_lr=1e-5)
]
history = model.fit(
    x_train, y_train,
    validation_split=0.1,
    epochs=50,
    batch_size=32,
    callbacks=callbacks,
    verbose=1
)

h5_path = MODEL_DIR / "crack_model.h5"
model.save(h5_path)

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()
tfl_path = MODEL_DIR / "crack_model.tflite"
tfl_path.write_bytes(tflite_model)

def bytes_to_c_array(b: bytes, var_name: str) -> str:
    hex_bytes = [f"0x{byte:02x}" for byte in b]
    lines = []
    for i in range(0, len(hex_bytes), 12):
        lines.append(", ".join(hex_bytes[i:i+12]))
    body = ",\n  ".join(lines)
    return f"""#ifndef MODEL_DATA_H
#define MODEL_DATA_H
#include <stddef.h>
#include <stdint.h>
alignas(4) const unsigned char {var_name}[] = {{
  {body}
}};
const unsigned int {var_name}_len = {len(b)};
#endif
"""

hdr = bytes_to_c_array(tflite_model, "crack_model_tflite")
(MODEL_DIR / "model_data.h").write_text(hdr, encoding="utf-8")

test_loss, test_acc = model.evaluate(x_test, y_test, verbose=0)
print(f"{CSV_PATH.name} | test_acc={test_acc:.4f} loss={test_loss:.4f}")
