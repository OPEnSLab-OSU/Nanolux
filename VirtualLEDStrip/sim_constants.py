# State constants
CONNECTING_STATE = 0
CONNECTED_STATE = 1
DISCONNECTED_STATE = 2
EXITING_STATE = 3
state_to_string = [
    'Connecting',
    'Connected',
    'Disconnected',
    'Exiting'
]
DEFAULT_STATE = DISCONNECTED_STATE

# UI Elements
CONNECT_BUTTON = "Connect"
DISCONNECT_BUTTON = "Disconnect"
STATUS_FIELD = "-STATUS-"
HSV_KEY = "HSV"
BGR_KEY = "BGR"
RGB_KEY = "RGB"
LOGO_SCALE = 10
LOGO_RELATIVE_PATH = '\\assets\logo2.png'
APP_NAME = "Nanolux LED Strip Simulator"
APP_THEME = 'SystemDefault'

# Numpy Constants
DEFAULT_LED_HEIGHT = 1
DEFAULT_LED_WIDTH = 60
LED_BAR_TARGET_LENGTH = 600

# Debug
NO_DEBUG = False
DEBUG = True
