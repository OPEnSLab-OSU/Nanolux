import sim_constants as sc
import numpy as np
import PySimpleGUI as sg
import sim_helpers as sim
from Audiolux import Nanolux

# Convert the logo's relative path to this file to an
# absolute system path.
logo_path = sim.get_absolute_path(sc.LOGO_RELATIVE_PATH)

# Convert the Windows' icon relative path to a system path.
icon_path = sim.get_absolute_path(sc.ICON_RELATIVE_PATH)

old_serial_options = [sc.NO_SERIAL_PORT] + sim.ports_to_list()
if "COM1" in old_serial_options:
    old_serial_options.remove("COM1")

# Configure the GUI
sg.theme(sc.APP_THEME)
layout = [
    [
        sg.Image(logo_path, subsample=sc.LOGO_SCALE),
        sg.OptionMenu(
            values=old_serial_options,
            key=sc.COM_SELECTOR_KEY,
            default_value=sc.NO_SERIAL_PORT,
        ),
        sg.Button(sc.RESCAN_BUTTON, key=sc.RESCAN_BUTTON),
    ],
    sim.make_display_bar(sc.HSV_KEY),
    sim.make_display_bar(sc.RGB_KEY),
    sim.make_display_bar(sc.BGR_KEY),
    [
        sg.Button(sc.CONNECT_BUTTON, key=sc.CONNECT_BUTTON),
        sg.Button(sc.DISCONNECT_BUTTON, key=sc.DISCONNECT_BUTTON),
        sg.Text(sc.state_to_string[sc.DEFAULT_STATE], key=sc.STATUS_FIELD)
    ],
]

# Create the Window
window = sg.Window(
    sc.APP_NAME,
    layout,
    icon=sim.path_to_png(icon_path),
)

# Ensure the openCV arrays are global variables
bgr = np.zeros((sc.DEFAULT_LED_HEIGHT, sc.LED_BAR_TARGET_LENGTH, 3), np.uint8)
hsv, rgb = bgr, bgr

# Update the application window with new images.
def update_bars(bgr, hsv, rgb):
    window[sc.RGB_KEY].update(data=sim.mat_to_png(rgb))
    window[sc.BGR_KEY].update(data=sim.mat_to_png(bgr))
    window[sc.HSV_KEY].update(data=sim.mat_to_png(hsv))

def update_selector_options():
    options = [sc.NO_SERIAL_PORT] + sim.ports_to_list()
    if "COM1" in options:
        options.remove("COM1")
    window[sc.COM_SELECTOR_KEY].update(values=options, value=sc.NO_SERIAL_PORT)

def disconnect_teardown(SER):
    SER.close()
    SER = None
    return sc.DISCONNECTED_STATE

if __name__ == '__main__':

    # Application state setup
    app_state = sc.DEFAULT_STATE
    SER = None
    connection_attempted = False
    device = None
    

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
        
        if event == sc.RESCAN_BUTTON:
            update_selector_options()

        # Reset the bgr image
        bgr = np.zeros((sc.DEFAULT_LED_HEIGHT, sc.LED_BAR_TARGET_LENGTH, 3), np.uint8)  # (B, G, R) 

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
                SER = sim.serial_setup(
                    sc.NO_DEBUG,
                    values[sc.COM_SELECTOR_KEY],
                    sc.TIMEOUT_LEN
                )

                if SER != None:
                    device = Nanolux(SER, sc.TIMEOUT_LEN)
                    app_state = sc.CONNECTED_STATE
                    print("Connected to NanoLux device.")
                    connection_attempted = False

            case sc.CONNECTED_STATE:
                # Set the disable state of both buttons
                window[sc.CONNECT_BUTTON].update(disabled=True)
                window[sc.DISCONNECT_BUTTON].update(disabled=False)

                # Check to see if the serial connection is still alive.
                # If it is not, move to the disconnect state.
                if device.is_running():
                    data = device.getBuffer()
                    bgr = sim.update_image(bgr, serial_data)
                    hsv, rgb = sim.bgr_to_hsv_rgb(bgr)
                    update_bars(bgr, hsv, rgb)




                try:
                    # Read in data from serial and process it into a bgr image.
                    serial_data = SER.readline().decode("utf-8").split()
                    bgr = sim.update_image(bgr, serial_data)
                    hsv, rgb = sim.bgr_to_hsv_rgb(bgr)
                    update_bars(bgr, hsv, rgb)
                except:
                    app_state = disconnect_teardown(SER)
                    update_selector_options()
                
                # If the connection button is pressed again, disconnect.
                if event == sc.DISCONNECT_BUTTON:
                    app_state = disconnect_teardown(SER)

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