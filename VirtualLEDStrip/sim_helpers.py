import cv2
import serial
import serial.tools.list_ports
import PySimpleGUI as sg
from PIL import Image
import io
import pathlib
import math
import sim_constants as sc

# Returns the size of each LED pixel given LED strip length
# and the amount of overhang pixels.
def calculate_pixel_width(led_count):
    w = math.floor(sc.LED_BAR_TARGET_LENGTH/led_count)
    e = sc.LED_BAR_TARGET_LENGTH - w*led_count
    return w, e

# Convert a relative path to an absolute path
def get_absolute_path(relative_path):
    return pathlib.Path(__file__).parent.resolve().__str__() + relative_path

# Generate UI elements for the LED strip bar and title string
def make_display_bar(key):
    return [sg.Text(key + ": "), sg.Image('', key=key)]

# Return the list of serial ports connected to the computer.
def serial_ports():
    ports = serial.tools.list_ports.comports()
    return ports

# Print the list of availible serial ports
def print_ports(ports):
    for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))

# Return a string list of all serial ports.
def ports_to_list():
    ports = serial_ports()
    str_ports = []
    for port, desc, hwid in sorted(ports):
        str_ports.append(port)
    return str_ports

# Set up and connect to serial port 4.
def serial_setup(is_debug, serial_port):
    available_ports = serial_ports() # Get all serial ports(com)
    if is_debug: print_ports(available_ports)
    try:
        SER = serial.Serial(serial_port, 115200)
    except:
        SER = None
    return SER

# Resizes an OpenCV image by a given scale factor.
# Typically used to scale a nx1 image to a new height.
def resize_image(img, scale_x, scale_y):
    width = int(img.shape[1] * scale_x)
    height = int(img.shape[0] * scale_y)
    dim = (width, height)
    resized = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
    return resized

# Attempts to update an OpenCV image with new serial data.
# If this fails, just scale the (probably black) image to
# the correct height.
def update_image(img, serial_data):
    # Get the number of pixels per pixel.
    try:
        w, _ = calculate_pixel_width(len(serial_data))
    # If this fails, the serial data is likely empty, so just resize
    # and return.
    except:
        img = resize_image(img, 1, 30)
        return img

    # Enumerate through the serial data and split out each pixel.
    for i, led_value in enumerate(serial_data):
        values = led_value.split(",")
        try:
            # Apply the recieved pixel color to all corresponding image
            # pixels.
            for j in range(i*w, (i+1)*w):
                img[:, j] = (int(values[2]), int(values[1]), int(values[0]))  # (B, G, R)
        except:
            pass
    img = resize_image(img, 1, 30)
    return img

# Citation:
# https://www.blog.pythonlibrary.org/2021/02/16/creating-an-image-viewer-with-pysimplegui/
def mat_to_png(mat):
    img = Image.fromarray(mat)
    bio = io.BytesIO()
    img.save(bio, format="PNG")
    return bio.getvalue()

def path_to_png(path):
    img = Image.open(path)
    bio = io.BytesIO()
    img.save(bio, format="PNG")
    return bio.getvalue()

# Convert a BGR image to an HSV and an RGB image.
def bgr_to_hsv_rgb(bgr):
    hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)
    rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
    return hsv, rgb
    