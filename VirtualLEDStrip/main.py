import types
import cv2
import numpy as np
import serial
import serial.tools.list_ports

# Application states and their IDs.
states = types.SimpleNamespace()
states.CONNECTING_STATE = 0
states.CONNECTED_STATE = 1
states.DISCONNECTED_STATE = 2
states.EXITING_STATE = 3

def serial_ports():
    ports = serial.tools.list_ports.comports()
    return ports


def print_ports(ports):
    for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))


def serial_setup():
    available_ports = serial_ports() # Get all serial ports(com)
    print_ports(available_ports)

    try:
        SER = serial.Serial("COM3", 115200)
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


if __name__ == '__main__':

    # Application state setup
    app_state = states.CONNECTING_STATE
    LED_HEIGHT = 1
    LED_WIDTH = 63
    SER = None
    connection_attempted = False

    # This image stores the BGR data for the application to display.
    blank_image = np.zeros((LED_HEIGHT, LED_WIDTH, 3), np.uint8)  # (B, G, R)

    # Main application loop
    while app_state != states.EXITING_STATE:
        match app_state:
            case states.CONNECTING_STATE:
                # If this is the first round of connections, give a notification
                # print to console.
                if not connection_attempted:
                    print("Connecting")
                    connection_attempted = True

                # Attempt to connect to serial. If serial_setup returns a value
                # that is not None, move to the connected state and give a
                # notification print.
                SER = serial_setup()
                if SER != None:
                    app_state = states.CONNECTED_STATE
                    print("Connected to NanoLux device.")
                    connection_attempted = False

            case states.CONNECTED_STATE:
                # Try to grab serial data and transform the blank image using the BGR
                # data from the AudioLux.
                # If this fails for whatever reason, move to the disconnected state.
                try:
                    serial_data = SER.readline().decode("utf-8").split()
                    blank_image = update_image(blank_image, serial_data)
                except:
                    app_state = states.DISCONNECTED_STATE

            case states.DISCONNECTED_STATE:
                # If we are in the disconnected state, give a notification print, then
                # immediately move to the connecting state.
                print("Device Disconnected")
                app_state = states.CONNECTING_STATE
            case _:
                # If we are in this state, there is an issue with state control.
                # If this print occurs, there is a code issue that must be resolved.
                print("Error, closing.")
                app_state = states.EXITING_STATE

        # Generates images based on different LED strip color arrangements.        
        blank_image = resize_image(blank_image, 30, 10)
        hsv_img = cv2.cvtColor(blank_image, cv2.COLOR_BGR2HSV)
        rgb_img = cv2.cvtColor(blank_image, cv2.COLOR_BGR2RGB)

        cv2.imshow('LED HSV', hsv_img)
        cv2.imshow('LED BGR', blank_image)
        cv2.imshow('LED RGB', rgb_img)
        cv2.waitKey(27)
    
    cv2.destroyAllWindows()
