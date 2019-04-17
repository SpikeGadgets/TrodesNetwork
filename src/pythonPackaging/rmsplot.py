#Ctrl+Shift+R to run highlighted code

#Libraries
from spikegadgets import trodesnetwork as tnp

import numpy
from matplotlib import pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons

#Define this for breaking loop, comes in later
stillrunning = True
def stoploop():
    global stillrunning
    stillrunning = False

#Definition of network client class
class PythonClient(tnp.AbstractModuleClient):
    def recv_quit(self):
        stoploop()
    def recv_event(self, origin, event, msg):
        print(origin, " ", event)
        if event == "NtrodeChanSelect":
            print(msg)

#Connect and initialize
network = PythonClient("RMSPlotter", "tcp://127.0.0.1", 49152)
if network.initialize() != 0:
    print("Network could not successfully initialize")
    del network
    quit()

datastream = network.subscribeNeuralData(1000, ['1, *', '2, *', '2, * ', '3, 0'])
datastream.initialize() #Initialize the streaming object
buf = datastream.create_numpy_array() #Assign 'buf' as a numpy array that is tied to the stream object

histbuf = numpy.zeros( (30000, buf.shape[0]) )

timestamp = 0
i = 0
fig, ax = plt.subplots()
rms = plt.bar(range(buf.shape[0]), numpy.zeros(buf.shape[0]))
# r = numpy.std(histbuf, axis=0)
for j in range(len(rms)):
    rms[j].set_height(r[j])

# plt.xlim(-1, 16)
plt.ylim(0, 100)
labels =  datastream.getChannelsRequested() 
ax.set_xticks(range(len(buf)))
ax.set_xticklabels(labels)
plt.show(False)


#Continuously get data until Trodes tells us to quit
while stillrunning:
    #Get the number of data points that came in
    n = datastream.available(1000) #timeout in ms
    #Go through and grab all the data packets, processing one at a time
    for z in range(n):
        timestamp = datastream.getData()
        histbuf[i] = buf 
        i += 1
        if i == 30000:
            i = 0
            r = numpy.std(histbuf, axis=0)*(0.1950103)
            for j in range(len(rms)):
                rms[j].set_height(r[j])
            fig.canvas.draw()
            # print(timestamp.trodes_timestamp, buf)


#Cleanup
network.closeConnections()
print("closed connections")
quit()
