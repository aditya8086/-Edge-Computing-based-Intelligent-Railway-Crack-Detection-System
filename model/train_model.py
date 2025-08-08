import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.preprocessing import LabelEncoder
from sklearn.model_selection import train_test_split

print("Imports successful")

# Load dataset
df = pd.read_csv(r'C:\Users\adity\railway crack detection\data\data\crack_depth_dataset.csv')
print("Dataset loaded")

# Encode labels
le = LabelEncoder()
df['label_encoded'] = le.fit_transform(df['label'])
print("Labels encoded")

# Split
x = df[['depth_cm']].values.astype(np.float32)
y = df['label_encoded'].values
x_train, x_test, y_train, y_test = train_test_split(x, y, test_size=0.2, random_state=42)
print("Data split")

# Define model
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(1,)),
    tf.keras.layers.Dense(64, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax')
])
model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
print("Model defined")

# Train model
model.fit(x_train, y_train, epochs=10, batch_size=4, verbose=1)
print("Model trained")

# Save .h5 model
model.save('crack_model.h5')
print("Saved as crack_model.h5")

# Convert to TFLite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()
print("Converted to TFLite")

# Save TFLite model
with open('crack_model.tflite', 'wb') as f:
    f.write(tflite_model)
print("Saved crack_model.tflite")
