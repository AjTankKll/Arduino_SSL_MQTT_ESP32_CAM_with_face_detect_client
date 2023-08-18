import paho.mqtt.client as mqtt
from PIL import Image  
import cv2
import time
import datetime
import os

pictrueCount = 0
fileCount = datetime.datetime.now().strftime('%H_%M_%S')
os.mkdir("C:\\Users\\q\\Pictures\\" + fileCount)
filePath = "C:\\Users\\q\\Pictures\\" + fileCount + "\\" + str(pictrueCount) + ".jpg"

img_received = False

def on_connect(client, userdata, flags, rc):
    print("Connected with -> "+str(rc))
    client.subscribe("camera")

def on_message(client, userdata, msg):
    global pictrueCount
    print("received message")
    message = msg.payload
    try:
        filePath = "C:\\Users\\q\\Pictures\\" + fileCount + "\\" + str(pictrueCount) + ".jpg"
        with open(filePath, "wb") as fh:
            fh.write(message)
        pictrueCount += 1
    except Exception as ret:
        print("error")

    global img_received
    img_received = True

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# SSL CA set
# client.tls_set(ca_certs='C:\\Users\\q\\Downloads\\emqxsl-ca.crt')
# client.username_pw_set(username="Your username",password="Your password")
client.connect("Your broker", 1883, 60)

face_engine = cv2.CascadeClassifier("C:\\Users\\q\\AppData\\Local\\Programs\\Python\\Python311\\Lib\\site-packages\\cv2\\data\\haarcascade_frontalface_default.xml") 
client.loop_start()

while True:
    time.sleep(0.1)

    try:
        if img_received:
            print("Image received")
            img_received = False
            filePath = "C:\\Users\\q\\Pictures\\" + fileCount + "\\" + str(pictrueCount - 1) + ".jpg"
            print(filePath)
            img = cv2.imread(filePath, 1)
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            faces = face_engine.detectMultiScale(img, scaleFactor=1.3, minNeighbors=2, minSize=(32, 32))
            
            if len(faces):
                for face in faces:
                    x, y, w, h = face
                    cv2.rectangle(img, (x, y), (x + h, y + w), (0,255,0), 2)
                    cv2.imwrite(filePath, img)
            cv2.imshow("mqtt_stream_shower",img)
            cv2.waitKey(1)
            # if cv2.waitKey(1) & 0xFF == ord('q'):
            #     break
    except Exception as e:
        print(e)