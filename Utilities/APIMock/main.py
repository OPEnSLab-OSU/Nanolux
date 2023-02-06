from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from data_model import Settings, Pattern, Control

app = FastAPI()

origins = [
    "http://localhost:8080",
    "http://localhost:8000",
    "http://localhost",
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


@app.get("/api/settings")
def get_settings():
    return settings


@app.put("/api/settings")
def put_settings(settings: Settings):
    return {"message": "Settings saved."}


@app.get("/api/patterns")
def get_pattern_list():
    return [pattern.value for pattern in Pattern]


@app.get("/api/pattern")
def get_current_pattern():
    return current_pattern


@app.put("/api/pattern")
def put_current_pattern(request: Control):
    global current_pattern
    current_pattern = request.pattern
    print(f"New pattern: {current_pattern}")
    return {"message": "Pattern saved."}
