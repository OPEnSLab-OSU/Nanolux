import sim_constants as sc
import numpy as np
import PySimpleGUI as sg
import sim_helpers as sim

# Convert the logo's relative path to this file to an
# absolute system path.
logo_path = sim.get_absolute_path(sc.LOGO_RELATIVE_PATH)

# Configure the GUI
sg.theme(sc.APP_THEME)
layout = [
    [sg.Image(logo_path, subsample=sc.LOGO_SCALE)],
    sim.make_display_bar(sc.HSV_KEY),
    sim.make_display_bar(sc.RGB_KEY),
    sim.make_display_bar(sc.BGR_KEY),
    [
        sg.Button(sc.CONNECT_BUTTON, key=sc.CONNECT_BUTTON),
        sg.Button(sc.DISCONNECT_BUTTON, key=sc.DISCONNECT_BUTTON),
        sg.Text(sc.state_to_string[sc.DEFAULT_STATE], key=sc.STATUS_FIELD)
    ]
]

# Create the Window
window = sg.Window(sc.APP_NAME, layout)

# Ensure the openCV arrays are global variables
bgr = np.zeros((sc.DEFAULT_LED_HEIGHT, sc.LED_BAR_TARGET_LENGTH, 3), np.uint8)
hsv, rgb = bgr, bgr

# Update the application window with new images.
def update_bars(bgr, hsv, rgb):
    window[sc.RGB_KEY].update(data=sim.mat_to_png(rgb))
    window[sc.BGR_KEY].update(data=sim.mat_to_png(bgr))
    window[sc.HSV_KEY].update(data=sim.mat_to_png(hsv))

if __name__ == '__main__':

    # Application state setup
    app_state = sc.DEFAULT_STATE
    SER = None
    connection_attempted = False

    # Ensure that the bars are drawn at least once
    event, values = window.read(0)
    blank = np.zeros((sc.DEFAULT_LED_HEIGHT, sc.DEFAULT_LED_WIDTH, 3), np.uint8)  # (B, G, R)   
    blank = sim.resize_image(blank, 10, 30)
    update_bars(blank, blank, blank)

    # Main application loop
    while app_state != sc.EXITING_STATE:

        # Check for events from the UI.
        event, values = window.read(0)
        if event == sg.WIN_CLOSED: # if user closes window or clicks cancel
            app_state = sc.EXITING_STATE

        # Reset the bgr image
        bgr = np.zeros((sc.DEFAULT_LED_HEIGHT, sim.LED_BAR_TARGET_LENGTH, 3), np.uint8)  # (B, G, R)

        match app_state:
            case sc.CONNECTING_STATE:
                # Set the disable state of both buttons
                window[sc.CONNECT_BUTTON].update(disabled=True)
                window[sc.DISCONNECT_BUTTON].update(disabled=False)
  
                # If the connection button is pressed again, disconnect.
                if event == sc.DISCONNECT_BUTTON:
                    app_state = sc.DISCONNECTED_STATE

                # Attempt to connect to serial. If serial_setup returns a value
                # that is not None, move to the connected state and give a
                # notification print.
                SER = sim.serial_setup(sim.NO_DEBUG)
                if SER != None:
                    app_state = sc.CONNECTED_STATE
                    print("Connected to NanoLux device.")
                    connection_attempted = False

            case sc.CONNECTED_STATE:
                # Set the disable state of both buttons
                window[sc.CONNECT_BUTTON].update(disabled=True)
                window[sc.DISCONNECT_BUTTON].update(disabled=False)

                # If the connection button is pressed again, disconnect.
                if event == sc.DISCONNECT_BUTTON:
                    app_state = sc.DISCONNECTED_STATE

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
                    app_state = sc.DISCONNECTED_STATE
                    SER.close()
                    SER = None

            case sc.DISCONNECTED_STATE:
                # Set the disable state of both buttons
                window[sc.CONNECT_BUTTON].update(disabled=False)
                window[sc.DISCONNECT_BUTTON].update(disabled=True)

                # If we are in the disconnected state, wait until the connect button
                # is pressed before moving to the connecting state.
                if event == sc.CONNECT_BUTTON:
                    app_state = sc.CONNECTING_STATE
                    
            case _:
                # If we are in this state, there is an issue with state control.
                # If this print occurs, there is a code issue that must be resolved.
                app_state = sc.EXITING_STATE
        
        # Update the graphs on the UI that display the virtual LED strips
        # if not in the process of exiting.
        if app_state != sc.EXITING_STATE:
            window[sc.STATUS_FIELD].update(sc.state_to_string[app_state])