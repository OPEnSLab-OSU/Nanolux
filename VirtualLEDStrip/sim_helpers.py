import cv2
import serial
import serial.tools.list_ports
import PySimpleGUI as sg
from PIL import Image
import io

NO_DEBUG = False
DEBUG = True

def make_display_bar(key):
    return [sg.Text(key + ": "), sg.Image(key="-" + key + "-")]

def serial_ports():
    ports = serial.tools.list_ports.comports()
    return ports

def print_ports(ports):
    for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))

def serial_setup(is_debug):
    available_ports = serial_ports() # Get all serial ports(com)
    if is_debug: print_ports(available_ports)
    try:
        SER = serial.Serial("COM4", 115200)
    except:
        SER = None
    return SER

def resize_image(img, scale_x, scale_y):
    width = int(img.shape[1] * scale_x)
    height = int(img.shape[0] * scale_y)
    dim = (width, height)
    resized = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
    return resized

def update_image(img, serial_data):
    led_values = []
    for i, led_value in enumerate(serial_data):
        values = led_value.split(",")
        try:
            img[:, i] = (int(values[2]), int(values[1]), int(values[0]))  # (B, G, R)
        except:
            pass
    return img

# Citation:
# https://www.blog.pythonlibrary.org/2021/02/16/creating-an-image-viewer-with-pysimplegui/
def mat_to_png(mat):
    img = Image.fromarray(mat)
    bio = io.BytesIO()
    img.save(bio, format="PNG")
    return bio.getvalue()