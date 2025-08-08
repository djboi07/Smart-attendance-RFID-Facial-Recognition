import socket
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.support.ui import Select
from selenium.webdriver.common.action_chains import ActionChains

import time
import requests
import cv2
import numpy as np
import os

# Configuration
DEVICE_IP = "192.168.1.100"  # Replace with your ESP32-CAM IP
PORT = 8080
CHROMEDRIVER_PATH = "path/to/chromedriver"  # Replace with your ChromeDriver path
IMAGE_DIR = "images"

# Face recognition function
def face_recog(reference_photo, captured_image):
    reference_gray = cv2.cvtColor(reference_photo, cv2.COLOR_BGR2GRAY)
    captured_gray = cv2.cvtColor(captured_image, cv2.COLOR_BGR2GRAY)
    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
    faces_ref = face_cascade.detectMultiScale(reference_gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    faces_captured = face_cascade.detectMultiScale(captured_gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    global match
    match = False
    if len(faces_ref) == 1 and len(faces_captured) == 1:
        x_ref, y_ref, w_ref, h_ref = faces_ref[0]
        face_ref = reference_gray[y_ref:y_ref+h_ref, x_ref:x_ref+w_ref]
        x_captured, y_captured, w_captured, h_captured = faces_captured[0]
        face_captured = captured_gray[y_captured:y_captured+h_captured, x_captured:x_captured+w_captured]
        result = cv2.matchTemplate(captured_gray, face_ref, cv2.TM_CCOEFF_NORMED)
        if cv2.minMaxLoc(result)[1] > 0.50:
            match = True

# Function to capture and compare face images
def capture_compare():
    chrome_options = Options()
    chrome_options.add_argument("--headless")
    driver = webdriver.Chrome(executable_path=CHROMEDRIVER_PATH, options=chrome_options)
    driver.get(f"http://{DEVICE_IP}")
    time.sleep(0.25)
    slider = driver.find_element(By.ID, "led_intensity")
    driver.execute_script("arguments[0].value = '255'", slider)
    ActionChains(driver).move_to_element(slider).click().perform()
    time.sleep(0.25)
    frame_size_dropdown = Select(driver.find_element(By.ID, "framesize"))
    frame_size_dropdown.select_by_value("10")
    time.sleep(0.25)
    driver.find_element(By.ID, "get-still").click()
    image_container = driver.find_element(By.ID, "stream-container")
    image_src = image_container.find_element(By.TAG_NAME, "img").get_attribute("src")
    response = requests.get(image_src)
    driver.quit()
    image_array = np.asarray(bytearray(response.content), dtype=np.uint8)
    captured_image = cv2.imdecode(image_array, -1)
    ref_path = os.path.join(IMAGE_DIR, msg, "sample1.jpg")
    reference_image = cv2.imread(ref_path)
    face_recog(captured_image, reference_image)
    files = os.listdir(os.path.join(IMAGE_DIR, msg))
    name = next((i[:-4] for i in files if i.endswith(".txt")), "Unknown")
    data = "hereT " + name if match else "hereF " + name
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as conn:
        conn.connect((DEVICE_IP, PORT))
        conn.send(data.encode())
        time.sleep(1)

# Main server loop
msg = ""
while True:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as conn:
        conn.connect((DEVICE_IP, PORT))
        msg = conn.recv(1024).decode()
        if len(msg) > 1:
            capture_compare()