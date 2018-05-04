from spikegadgets import trodesnetwork as tnp
import numpy
#Define this for breaking loop, comes in later
stillrunning = True
def stoploop():
    global stillrunning
    stillrunning = False


#Definition of network client class
class PythonClient(tnp.AbstractModuleClient):
    def recv_quit(self):
        stoploop()


#Connect and initialize
network = PythonClient("TestPython", "tcp://127.0.0.1",49152)
if network.initialize() != 0:
    print("Network could not successfully initialize")
    del network
    quit()

datastream = network.subscribeLFPData(100, ['1','2', '5', '6', '7', '8', '10'])
datastream.initialize() #Initialize the streaming object

buf = datastream.create_numpy_array()
timestamp = 0
#Continuously get data until Trodes tells us to quit, which triggers the function that flips the "stillrunning" variable
while stillrunning:
    #Get the number of data points that came in
    n = datastream.available(1000) #timeout in ms
    #Go through and grab all the data packets, processing one at a time
    for i in range(n):
        timestamp = datastream.getData() 
        print(buf)
#Cleanup
del network