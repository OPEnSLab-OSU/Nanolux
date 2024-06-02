# Mock API backend 
## For the AudioLux web application

Runs a barebones server conforming to the AudioLux API specification. 
Although it does not implement any behavior, it preserves states
during the session.

Requires Python 3. Implemented using 3.10, but could run with 
previous versions. It is strongly recommended to create and 
activate virtual environment before installing and running.

To create and activate a virtual environment:

```commandline
python3 -m venv venv
source venv/bin/activate
```

If running Windows:
```commandline
venv\Scripts\activate
```

Once activated, install the dependencies:
```commandline
pip3 install -f requirements.txt
```

To run the server:
```commandline
uvicorn main:app --proxy-headers --host 0.0.0.0 --port 8000
```

To stop it, press `ctrl-c`. To exit the virtual environment:
```commandline
deactivate
```
