# Edge Computing Based Intelligent Railway Crack Detection System

This project implements an intelligent railway crack detection system **using Edge AI** on the **ESP32-CAM microcontroller**, combining multiple sensors and deep learning to classify crack severity and alert authorities in real-time.

## Overview

**Ultrasonic sensor** measures the depth of cracks. A **TinyML model** is trained on depth values to classify severity.
If the crack is classified as **severe**: A photo is captured using the **ESP32-CAM**. The image is uploaded to a server via **Wi-Fi**. An **SMS alert** is sent with **GPS coordinates** and**Image link** for verification.

All computations happen **locally on the device**, making the system fast, offline-capable, and deployment-ready.


## Features


- Crack detection using trained ML model
- Critical crack classification using ultrasonic threshold
- Real-time image capture of detected cracks
- Upload image to remote server via Wi-Fi
- Send **SMS alert** with crack **severity + image link + GPS coordinates**
- Fully edge-deployable (no cloud inference)
- Modular firmware (Arduino IDE)


## Hardware Used

| Component       | Description                                  |
|----------------|----------------------------------------------|
| ESP32-CAM       | Microcontroller with built-in camera module  |
| Ultrasonic Sensor | Detects physical crack depth/width         |
| SIM800L         | GSM module to send SMS alerts                |
| GPS Module (Neo-6M) | Provides GPS coordinates for alerts     |
| Power Supply    | 5V/2A battery or adapter                     |


## Software Stack

**Arduino IDE** for firmware
**Python + TensorFlow** for training the crack detection model
**TinyML (TFLite)** for on-device inference
**PHP** script ('upload.php') for image uploads
**Git** + **GitHub** for version control and hosting



