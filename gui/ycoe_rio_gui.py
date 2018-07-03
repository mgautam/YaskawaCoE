#import json
import struct
import zmq
from kivy.app import App
from kivy.config import Config
from kivy.core.window import Window
from kivy.uix.button import Button
from kivy.uix.floatlayout import FloatLayout
from kivy.clock import Clock

class AbsMoveBtn(Button):
   def set_acceleration(self, slavenum, straccel):
        ctrlwindow=self.parent.parent.parent.parent
        accel=int(straccel)
        if accel>4294967295:
            accel=4294967295
        elif accel<0:
            accel=0
        ctrlwindow.socket.send(struct.pack('<BBI',39,slavenum,accel))
        message = ctrlwindow.socket.recv()

   def set_velocity(self, slavenum, strvelocity):
        ctrlwindow=self.parent.parent.parent.parent
        velocity=int(strvelocity)
        if velocity>4294967295:
            velocity=4294967295
        elif velocity<0:
            velocity=0
        ctrlwindow.socket.send(struct.pack('<BBI',36,slavenum,velocity))
        message = ctrlwindow.socket.recv()

   def move_absolute(self, distance):
        ctrlwindow=self.parent.parent.parent
        #cmdmsg=json.dumps({"type":"absolute","distance":distance})
        #print([distance])
        ctrlwindow.socket.send(distance)
        message = ctrlwindow.socket.recv()
        #print("Received reply %s [ %s ]" % (cmdmsg, message))

class CmdPosBtn(Button):
    def move_cmdpos(self, slavenum, strposition):
        ctrlwindow=self.parent.parent.parent.parent
        distance=int(strposition)
        if distance>4294967295:
            distance=4294967295
        elif distance<0:
            distance=0
        #cmdmsg=json.dumps({"type":"absolute","distance":distance})
        #print([distance, distance.to_bytes(4, byteorder='little'), struct.pack(distance])
        #print(struct.pack('<BI',3,distance))
        ctrlwindow.socket.send(struct.pack('<BBI',3,slavenum,distance))
        message = ctrlwindow.socket.recv()
        #print("Received reply %s [ %s ]" % (cmdmsg, message))

class StopBtn(Button):
    def stopaxis(self, slavenum):
        ctrlwindow=self.parent.parent.parent
        ctrlwindow.socket.send(struct.pack('<BB',42,slavenum))
        message = ctrlwindow.socket.recv()
    def stopAllaxes(self):
        ctrlwindow=self.parent.parent.parent
        ctrlwindow.socket.send(struct.pack('<BB',43,1))
        message = ctrlwindow.socket.recv()

class RegReadBtn(Button):
    def readreg(self, slaveaddr, regaddr):
        ctrlwindow=self.parent.parent.parent
        islaveaddr=int(slaveaddr)
        iregaddr=int(regaddr,16)
        if islaveaddr > 0xFFFF:
            islaveaddr = 0xFFFF
        if iregaddr > 0xFFFF:
            iregaddr = 0xFFFF
        ctrlwindow.socket.send(struct.pack('<BBI',6,islaveaddr,iregaddr))
        message = ctrlwindow.socket.recv()

class ReadCOBtn(Button):
    def readcoparam(self, slaveaddr, index, subindex):
        ctrlwindow=self.parent.parent.parent
        islaveaddr=int(slaveaddr)
        iindex=int(index,16)
        isubindex=int(subindex,16)
        if islaveaddr > 0xFFFF:
            islaveaddr = 0xFFFF
        if iindex > 0xFFFF:
            iindex = 0xFFFF
        ctrlwindow.socket.send(struct.pack('<BBII',9,islaveaddr,iindex,isubindex))
        message = ctrlwindow.socket.recv()

class MultiPosBtn(Button):
    def move_cmdpos(self, strposition):
        ctrlwindow=self.parent.parent.parent
        distance=int(strposition)
        if distance>4294967295:
            distance=4294967295
        elif distance<0:
            distance=0
        ctrlwindow.socket.send(struct.pack('<BI',33,distance))
        message = ctrlwindow.socket.recv()

class StsLabel(Button):
    def toggle_dout(self, slaveaddr, ionum):
        ctrlwindow=self.parent.parent.parent
        ctrlwindow.socket.send(struct.pack('<BBI',23,slaveaddr,ionum))
        message = ctrlwindow.socket.recv()

class controlWindow(FloatLayout):
    context=None
    socket=None
    def __init__(self, **kwargs):
        supret = super().__init__(**kwargs)
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect("tcp://localhost:5555")
        Clock.schedule_once(self._zmq_read, .9)
        self.statuslbl.text="Status: Couldn't Connect to Server! Please restart application..."
        return supret

    def _zmq_read(self, dt):
        try:
            self.socket.send_string("Request")
            data = self.socket.recv()
            numslaves=int.from_bytes(data[0:4],byteorder='little')
            self.statuslbl.text="Status: Slavecount = "+ str(numslaves)
            #frloc = framelocation
            frloc = 4
            inputbytes=int.from_bytes(data[frloc:frloc+4],byteorder='little')
            outputbytes=int.from_bytes(data[frloc+4:frloc+8],byteorder='little')
            idata = data[frloc+8:frloc+8+inputbytes]
            odata = data[frloc+8+inputbytes:frloc+8+inputbytes+outputbytes]

            self.errorcode1.text=str(hex(int.from_bytes(data[46:48],byteorder='little')))
            self.tarpos1.text=str(int.from_bytes(odata[2:6],byteorder='little'))
            self.curpos1.text=str(int.from_bytes(idata[2:6],byteorder='little'))
            self.statusword1.text=''.join('{:02x}'.format(x) for x in reversed(idata[0:2]))
            self.controlword1.text=''.join('{:02x}'.format(x) for x in reversed(odata[0:2]))

            #frloc = framelocation
            frloc = 12+inputbytes+outputbytes
            inputbytes=int.from_bytes(data[frloc:frloc+4],byteorder='little')
            outputbytes=int.from_bytes(data[frloc+4:frloc+8],byteorder='little')
            idata = data[frloc+8:frloc+8+inputbytes]
            odata = data[frloc+8+inputbytes:frloc+8+inputbytes+outputbytes]

            self.errorcode2.text=str(hex(int.from_bytes(data[48:50],byteorder='little')))
            self.tarpos2.text=str(int.from_bytes(odata[2:6],byteorder='little'))
            self.curpos2.text=str(int.from_bytes(idata[2:6],byteorder='little'))
            self.statusword2.text=''.join('{:02x}'.format(x) for x in reversed(idata[0:2]))
            self.controlword2.text=''.join('{:02x}'.format(x) for x in reversed(odata[0:2]))

            #frloc = framelocation
            frloc = 4+(8+inputbytes+outputbytes)*2
            inputbytes=int.from_bytes(data[frloc:frloc+4],byteorder='little')
            outputbytes=int.from_bytes(data[frloc+4:frloc+8],byteorder='little')
            idata = data[frloc+8:frloc+8+inputbytes]
            odata = data[frloc+8+inputbytes:frloc+8+inputbytes+outputbytes]
            self.digin.text=str(hex(int.from_bytes(idata[0:2],byteorder='little')))
            self.digout.text=str(hex(int.from_bytes(odata[0:4],byteorder='little')))
            #self.anin1.text=str(hex(int.from_bytes(idata[4:6],byteorder='little')))
            self.anin1.text=str("{0:.2f}".format(float(int.from_bytes(idata[4:6],byteorder='little',signed=True)*10)/32767))
            self.anin2.text=str("{0:.2f}".format(float(int.from_bytes(idata[6:8],byteorder='little',signed=True)*10)/32767))
            self.anin3.text=str("{0:.2f}".format(float(int.from_bytes(idata[8:10],byteorder='little',signed=True)*10)/32767))
            self.anin4.text=str("{0:.2f}".format(float(int.from_bytes(idata[10:12],byteorder='little',signed=True)*10)/32767))
            self.anin5.text=str("{0:.2f}".format(float(int.from_bytes(idata[12:14],byteorder='little',signed=True)*10)/32767))
            self.anin6.text=str("{0:.2f}".format(float(int.from_bytes(idata[14:16],byteorder='little',signed=True)*10)/32767))
            self.anin7.text=str("{0:.2f}".format(float(int.from_bytes(idata[16:18],byteorder='little',signed=True)*10)/32767))
            self.anin8.text=str("{0:.2f}".format(float(int.from_bytes(idata[18:20],byteorder='little',signed=True)*10)/32767))

            Clock.schedule_once(self._zmq_read, .9)
        except zmq.ZMQError:
            self.statuslbl.text="Status: Couldn't Connect to Server! Please restart application..."
            return

class ControlrioApp(App):
    def build(self):
        self.title = "YaskawaCoE Master"
        return controlWindow()

if __name__ == '__main__':
    Config.set('graphics', 'width',  1080)
    Config.set('graphics', 'height', 600)
    Window.size = (1080,600)
    ControlrioApp().run()
