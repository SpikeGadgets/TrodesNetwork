#Ctrl+Shift+R to run highlighted code

#Libraries
from spikegadgets import trodesnetwork as tnp
# import os
# os.chdir('/home/kevin/spikegadgets/TrodesNetwork/build/lib')
# import trodesnetwork as tnp
# print(tnp.__file__)

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
    def recv_event(self, origin, event, msg):
        print(origin, " ", event)
        if event == "NtrodeChanSelect":
            print(msg)


#Connect and initialize
network = PythonClient("BasicPython", "tcp://127.0.0.1",49152)
if network.initialize() != 0:
    print("Network could not successfully initialize")
    del network
    quit()

# network.subscribeToEvent("Trodes", "NtrodeChanSelect")


network.initializeHardwareConnection()

# network.sendSettleCommand()
# s = tnp.StimulationCommand()
# s.setGroup(1)
# s.setSlot(1)
# s.setChannels(1,2, 1,2)
# s.setBiphasicPulseShape(5, 5, 5, 5, 5, 20, 5)
# s.isValid()

# network.sendStimulationParams(s)
# network.sendClearStimulationParams(1)
# network.sendStimulationStartSlot(2)
# network.sendStimulationStartGroup(3)
# network.sendStimulationStopSlot(4)
# network.sendStimulationStopGroup(5)

# gs = tnp.GlobalStimulationSettings()
# gs.setVoltageScale(tnp.CurrentScaling.max20nA)
# network.sendGlobalStimulationSettings(gs)

# gc = tnp.GlobalStimulationCommand()
# gc.setStimEnabled(true)
# network.sendGlobalStimulationCommand(gc)

#Prints out available channels of Trodes data to subscribe to
# network.getAvailableTrodesData(tnp.datastream)
# network.getAvailableTrodesData(tnp.NEURALSTREAM)

################################################################################

# datastream = network.subscribeHighFreqData('PositionData', 'CameraModule', 60)
# datastream.initialize()
# ndtype = datastream.getDataType().dataFormat
# nbytesize = datastream.getDataType().byteSize
# dt = numpy.dtype(ndtype)
# buf = memoryview(bytes(nbytesize))
# npbuff = numpy.frombuffer(buf, dtype=dt)
# while stillrunning:
#     n = datastream.available(1000)
#     for i in range(n):
#         byteswritten = datastream.readData(npbuff)
#         print(tnp.systemTimeMSecs() - datastream.lastSysTimestamp(), npbuff[0])
#         # print(bytes(buf))

################################################################################

#Subscribe to LFP data, from NTrodes 1 and 2
# datastream = network.subscribeLFPData(100, ['1','2', '5', '6', '7', '8', '10'])
# datastream = network.subscribeLFPData(500, ["{:0d}".format(x) for x in range(1,31)])
# datastream = network.subscribeSpikesData(100, ['3, 0', '3,1', '8, 0'])
# datastream = network.subscribeAnalogData(100, ["ECU,Ain1", "headstageSensor,GyroX"])
# datastream = network.subscribeDigitalData(100, ["ECU,Din12", "ECU,Din13"])
datastream = network.subscribeNeuralData(1000, ['1,1', '1,2', '2,1', '2,2', '3,1'])
# datastream = network.subscribeNeuralData(1000, [str(i+1)+',1' for i in range(32)])
datastream.initialize() #Initialize the streaming object
# #Assign 'buf' as a numpy array that is tied to the stream object
buf = datastream.create_numpy_array()
timestamp = 0
#Continuously get data until Trodes tells us to quit
# while stillrunning:
grabbed = 0
import time
start = time.monotonic()
# while(grabbed <=1000000):
while stillrunning:
    #Get the number of data points that came in
    n = datastream.available(1000   ) #timeout in ms
    #Go through and grab all the data packets, processing one at a time
    # print(n)
    for i in range(n):
        timestamp = datastream.getData() 
        print(timestamp.trodes_timestamp, buf)
        # grabbed = grabbed + 1
        # print(numpy.sum(buf))
    # if grabbed >= 15000:
    #     print("time(sec): ", time.monotonic()-start)
    #     break


#Cleanup
network.closeConnections()
print("closed connections")
quit()
