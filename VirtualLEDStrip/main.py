import types
import cv2
import numpy as np
import PySimpleGUI as sg
import sim_helpers as sim

# Application states and their IDs.
states = types.SimpleNamespace()
states.CONNECTING_STATE = 0
states.CONNECTED_STATE = 1
states.DISCONNECTED_STATE = 2
states.EXITING_STATE = 3

LED_HEIGHT = 1
LED_WIDTH = 60
PIXEL_LENGTH = 14

# GUI Constants
LOGO_SCALE = 10
LOGO_PATH = 'assets/logo2.png'

sg.theme('SystemDefault')
layout = [
    [sg.Image(LOGO_PATH, subsample=LOGO_SCALE)],
    sim.make_display_bar("HSV"),
    sim.make_display_bar("BGR"),
    sim.make_display_bar("RGB")
]

# Create the Window
window = sg.Window('Window Title', layout)

if __name__ == '__main__':

    # Application state setup
    app_state = states.CONNECTING_STATE
    SER = None
    connection_attempted = False

    # Main application loop
    while app_state != states.EXITING_STATE:

        # This image stores the BGR data for the application to display.
        blank_image = np.zeros((LED_HEIGHT, LED_WIDTH, 3), np.uint8)  # (B, G, R)

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
                SER = sim.serial_setup(sim.NO_DEBUG)
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
                    blank_image = sim.update_image(blank_image, serial_data)
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
        blank_image = sim.resize_image(blank_image, 10, 30)
        hsv_img = cv2.cvtColor(blank_image, cv2.COLOR_BGR2HSV)
        rgb_img = cv2.cvtColor(blank_image, cv2.COLOR_BGR2RGB)

        event, values = window.read(0)
        if event == sg.WIN_CLOSED: # if user closes window or clicks cancel
            app_state = states.EXITING_STATE
            break

        window['-RGB-'].update(data=sim.mat_to_png(rgb_img))
        window['-BGR-'].update(data=sim.mat_to_png(blank_image))
        window['-HSV-'].update(data=sim.mat_to_png(hsv_img))

        
