import types
import numpy as np
import PySimpleGUI as sg
import sim_helpers as sim

# Application states and their IDs.
states = types.SimpleNamespace()
states.CONNECTING_STATE = 0
states.CONNECTED_STATE = 1
states.DISCONNECTED_STATE = 2
states.EXITING_STATE = 3
state_to_string = [
    'Connecting',
    'Connected',
    'Disconnected',
    'Exiting'
]

LED_HEIGHT = 1
LED_WIDTH = 60
PIXEL_LENGTH = 14
CONNECT_BUTTON = "-CONNECT-"
DISCONNECT_BUTTON = "-DISCONNECT-"

# GUI Constants
LOGO_SCALE = 10
LOGO_PATH = sim.get_absolute_path('\\assets\logo2.png')

sg.theme('SystemDefault')
layout = [
    [sg.Image(LOGO_PATH, subsample=LOGO_SCALE)],
    sim.make_display_bar("HSV"),
    sim.make_display_bar("BGR"),
    sim.make_display_bar("RGB"),
    [
        sg.Button('Connect', key=CONNECT_BUTTON),
        sg.Button('Disconnect', key=DISCONNECT_BUTTON),
        sg.Text('', key="-STATUS-")
    ]
]

# Create the Window
window = sg.Window('Window Title', layout)

# Update the application window with new images.
def update_bars(bgr, hsv, rgb):
    window['-RGB-'].update(data=sim.mat_to_png(rgb))
    window['-BGR-'].update(data=sim.mat_to_png(bgr))
    window['-HSV-'].update(data=sim.mat_to_png(hsv))

if __name__ == '__main__':

    # Application state setup
    app_state = states.DISCONNECTED_STATE
    SER = None
    connection_attempted = False

    # Defaults the LED strip length to 60.
    # Common LED strip length, divides well into
    # 600.
    strip_length = 60

    # Ensure that the bars are drawn at least once
    event, values = window.read(0)
    blank = np.zeros((LED_HEIGHT, LED_WIDTH, 3), np.uint8)  # (B, G, R)   
    blank = sim.resize_image(blank, 10, 30)
    update_bars(blank, blank, blank)

    # Main application loop
    while app_state != states.EXITING_STATE:

        # Check for events from the UI.
        event, values = window.read(0)
        if event == sg.WIN_CLOSED: # if user closes window or clicks cancel
            app_state = states.EXITING_STATE

        # This image stores the BGR data for the application to display.
        bgr = np.zeros((LED_HEIGHT, sim.LED_BAR_TARGET_LENGTH, 3), np.uint8)  # (B, G, R)

        match app_state:
            case states.CONNECTING_STATE:
                # Set the disable state of both buttons
                window[CONNECT_BUTTON].update(disabled=True)
                window[DISCONNECT_BUTTON].update(disabled=False)
  
                # If the connection button is pressed again, disconnect.
                if event == DISCONNECT_BUTTON:
                    app_state = states.DISCONNECTED_STATE

                # Attempt to connect to serial. If serial_setup returns a value
                # that is not None, move to the connected state and give a
                # notification print.
                SER = sim.serial_setup(sim.NO_DEBUG)
                if SER != None:
                    app_state = states.CONNECTED_STATE
                    print("Connected to NanoLux device.")
                    connection_attempted = False

            case states.CONNECTED_STATE:
                # Set the disable state of both buttons
                window[CONNECT_BUTTON].update(disabled=True)
                window[DISCONNECT_BUTTON].update(disabled=False)

                # If the connection button is pressed again, disconnect.
                if event == DISCONNECT_BUTTON:
                    app_state = states.DISCONNECTED_STATE

                # Try to grab serial data and transform the blank image using the BGR
                # data from the AudioLux.
                # If this fails for whatever reason, move to the disconnected state and
                # disconnect from the AudioLux.
                try:
                    # Read in data from serial and process it into a bgr image.
                    serial_data = SER.readline().decode("utf-8").split()
                    bgr = sim.update_image(bgr, serial_data)
                    hsv, rgb = sim.bgr_to_hsv_rgb(bgr)
                    update_bars(bgr, hsv, rgb)
                except:
                    app_state = states.DISCONNECTED_STATE
                    SER.close()
                    SER = None

            case states.DISCONNECTED_STATE:
                # Set the disable state of both buttons
                window[CONNECT_BUTTON].update(disabled=False)
                window[DISCONNECT_BUTTON].update(disabled=True)

                # If we are in the disconnected state, wait until the connect button
                # is pressed before moving to the connecting state.
                if event == CONNECT_BUTTON:
                    app_state = states.CONNECTING_STATE
                    
            case _:
                # If we are in this state, there is an issue with state control.
                # If this print occurs, there is a code issue that must be resolved.
                app_state = states.EXITING_STATE
        
        # Update the graphs on the UI that display the virtual LED strips
        # if not in the process of exiting.
        if app_state != states.EXITING_STATE:
            window['-STATUS-'].update(state_to_string[app_state])

        
