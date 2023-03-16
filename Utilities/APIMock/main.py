from fastapi import FastAPI
from fastapi.exceptions import RequestValidationError
from fastapi.middleware.cors import CORSMiddleware
from starlette import status
from starlette.requests import Request
from starlette.responses import JSONResponse

from data_model import (
    Settings,
    Pattern,
    Control,
    WiFiCreds,
    WiFi,
    Hostname,
    ApiResponse
)

app = FastAPI()

origins = [
    "http://localhost:8080",
    "http://localhost:8000",
    "http://localhost",
    "https://audioluxecampus.azurewebsites.net/",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"]
)

current_pattern = Pattern.trail
settings = Settings(
    noise=10,
    compression=90,
    loFreqHue=55,
    hiFreqHue=200,
    ledCount=50
)


history = []

wifis = [
    WiFi.parse_obj({
        "ssid": "WiFi1", "rssi": 20, "lock": False
    }),
    WiFi.parse_obj({
        "ssid": "WiFi2", "rssi": 80, "lock": True
    }),
    WiFi.parse_obj({
        "ssid": "WiFi3", "rssi": 100, "lock": False
    }),
    WiFi.parse_obj({
        "ssid": "WiFi4", "rssi": 50, "lock": True
    }),
    WiFi.parse_obj({
        "ssid": "Bad WiFi", "rssi": 90, "lock": True
    }),
]

selected_wifi = None

joined_wifi = None

current_hostname = Hostname.parse_obj({"hostname": "audiolux"})


@app.exception_handler(RequestValidationError)
async def validation_exception_handler(request: Request, exc: RequestValidationError):
    exc_str = f'{exc}'.replace('\n', ' ').replace('   ', ' ')
    print(f'{exc}\n{request}')
    # logger.error(request, exc_str)
    content = {'status_code': 10422, 'message': exc_str, 'data': None}
    return JSONResponse(content=content, status_code=status.HTTP_422_UNPROCESSABLE_ENTITY)


@app.get("/api/settings")
def get_settings():
    history.append(f"Retrieved settings: {settings}\n")
    return settings


@app.put("/api/settings")
def put_settings(new_settings: Settings):
    global settings
    settings = new_settings.copy()
    history.append(f"Saved settings: {settings}\n")
    return {"message": "Settings saved."}


@app.get("/api/patterns")
def get_pattern_list():
    history.append(f"Retrieved pattern list: {[p.name for p in Pattern]}\n")
    return [pattern.value for pattern in Pattern]


@app.get("/api/pattern")
def get_current_pattern():
    history.append(f"Retrieved current pattern: {current_pattern}\n")
    return current_pattern


@app.put("/api/pattern")
def put_current_pattern(request: Control) -> ApiResponse:
    global current_pattern
    current_pattern = request.pattern
    print(f"New pattern: {current_pattern}")
    history.append(f"Saved pattern: {current_pattern}\n")
    return ApiResponse(message="Pattern saved.", success=True)


@app.get("/api/wifis")
def get_wifi_list():
    history.append(f"Retrieved WiFi list: {[wifi.ssid for wifi in wifis]}\n")
    return [wifi for wifi in wifis]


@app.get("/api/wifi")
def get_current_wifi():
    history.append(f"Retrieved current wifi: {joined_wifi}\n")
    return joined_wifi


@app.put("/api/wifi")
def join_wifi(request: WiFiCreds) -> ApiResponse:
    global selected_wifi
    global joined_wifi
    selected_wifi = request.ssid

    if selected_wifi == "" and joined_wifi is not None:
        history.append(f"Forgetting wifi: {joined_wifi.ssid}")
        joined_wifi = None
        return ApiResponse(message=f"{joined_wifi.ssid} forgotten", success=True)

    if selected_wifi != "":
        history.append(f"Selected wifi: {selected_wifi}")

    success = selected_wifi != "Bad WiFi"
    entry = f"{selected_wifi} not joined"
    joined_wifi = None
    if success:
        joined_wifi = next((wifi for wifi in wifis if wifi.ssid == request.ssid), None)
        entry = f"{joined_wifi.ssid} joined"
        if request.key:
            entry += f", with key: {request.key}"

    print(entry)
    history.append(entry)

    return ApiResponse(message=entry, success=success)


@app.get("/api/hostname")
def get_current_hostname() -> Hostname:
    history.append(f"Retrieved current hostname: {current_hostname}\n")
    return current_hostname


@app.put("/api/hostname")
def set_current_hostname(hostname: Hostname) -> ApiResponse:
    global current_hostname
    current_hostname = hostname
    history.append(f"Setting current hostname to: {current_hostname.hostname}\n")
    return ApiResponse(
        message="Hostname changed.",
        details=f"Hostname is now {current_hostname.hostname}",
        success=True)


@app.get("/api/history")
def get_history():
    global history
    saved_history = history.copy()
    history = []
    return saved_history
