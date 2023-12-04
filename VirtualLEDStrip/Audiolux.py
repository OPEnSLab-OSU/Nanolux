from threading import Thread
import sim_constants as sc
import numpy as np
import PySimpleGUI as sg
import sim_helpers as sim
import time

class Nanolux:
    
    def __init__(self, serial, timeout):
        self.serial = serial
        self.buf = []
        self.timer = Timeout_Timer(timeout)
        self.thread = Thread(target=self.__mainloop)
        self.thread.start()
        self.kill = False
    
    # This is the main loop of the thread.
    def __mainloop(self):

        alive = True
        buf = []

        while(alive):

            self.timer.reset()
            try:
                buf = self.serial.readline().decode("utf-8").split()
            except:
                alive = False

            self.buf = buf
            
            # End the loop if the timeout is exceeded
            if(self.timer.aboveTimeout() or self.kill):
                alive = False
        
        self.serial.close()
    
    # Returns the LED strip buffer.
    def getBuffer(self):
        return self.buf
    
    # Checks if the thread is running.
    def is_running(self):
        return self.thread.is_alive()
    
    # Signals the thread to end. Joins the thread until it terminates.
    def kill(self):
        self.kill = True
        self.thread.join()


class Timeout_Timer:
    def __init__(self, timeout):
        self.timeout = timeout
        self.time = 0
    
    def reset(self):
        self.time = time.time()
    
    def aboveTimeout(self):
        return (time.time() - self.time) > self.timeout