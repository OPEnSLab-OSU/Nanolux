import cv2
import numpy as np
import serial
import serial.tools.list_ports

def serial_ports():
    ports = serial.tools.list_ports.comports()
    return ports


def print_ports(ports):
    for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))


def serial_setup():
    available_ports = serial_ports() # Get all serial ports(com)
    print_ports(available_ports)
    SER = serial.Serial("COM3", 115200)
    return SER


def resize_image(img, scale_x, scale_y):
    width = int(img.shape[1] * scale_x)
    height = int(img.shape[0] * scale_y)
    dim = (width, height)
    resized = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
    return resized

if __name__ == '__main__':
    LED_HEIGHT = 1
    LED_WIDTH = 63
    SER = serial_setup()
    while True:
        blank_image = np.zeros((LED_HEIGHT, LED_WIDTH, 3), np.uint8)  # (B, G, R)
        led_values = []
        serial_data = SER.readline().decode("utf-8").split()
        for i, led_value in enumerate(serial_data):
            values = led_value.split(",")
            try:
                blank_image[:, i] = (int(values[2]), int(values[1]), int(values[0]))  # (B, G, R)
            except:
                pass

        blank_image = resize_image(blank_image, 30, 10)
        hsv_img = cv2.cvtColor(blank_image, cv2.COLOR_BGR2HSV)
        rgb_img = cv2.cvtColor(blank_image, cv2.COLOR_BGR2RGB)

        cv2.imshow('LED HSV', hsv_img)
        cv2.imshow('LED BGR', blank_image)
        cv2.imshow('LED RGB', rgb_img)
        cv2.waitKey(27)
    cv2.destroyAllWindows()
