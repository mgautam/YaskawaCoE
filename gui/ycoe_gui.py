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
    def move_absolute(self, distance):
        ctrlwindow=self.parent.parent.parent
        ctrlwindow.socket.send(distance)
        message = ctrlwindow.socket.recv()

class CmdPosBtn(Button):
    def move_cmdpos(self, slavenum, strposition):
        ctrlwindow=self.parent.parent.parent
        distance=int(strposition)
        if distance>4294967295:
            distance=4294967295
        elif distance<0:
            distance=0
        ctrlwindow.socket.send(struct.pack('<BBI',3,slavenum,distance))
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

            self.tarpos1.text=str(int.from_bytes(odata[2:6],byteorder='little'))
            self.curpos1.text=str(int.from_bytes(idata[2:6],byteorder='little'))
            self.statusword1.text=''.join('{:02x}'.format(x) for x in reversed(idata[0:2]))
            self.controlword1.text=''.join('{:02x}'.format(x) for x in reversed(odata[0:2]))

            frloc = 12+inputbytes+outputbytes
            inputbytes=int.from_bytes(data[frloc:frloc+4],byteorder='little')
            outputbytes=int.from_bytes(data[frloc+4:frloc+8],byteorder='little')
            idata = data[frloc+8:frloc+8+inputbytes]
            odata = data[frloc+8+inputbytes:frloc+8+inputbytes+outputbytes]

            self.tarpos2.text=str(int.from_bytes(odata[2:6],byteorder='little'))
            self.curpos2.text=str(int.from_bytes(idata[2:6],byteorder='little'))
            self.statusword2.text=''.join('{:02x}'.format(x) for x in reversed(idata[0:2]))
            self.controlword2.text=''.join('{:02x}'.format(x) for x in reversed(odata[0:2]))

            Clock.schedule_once(self._zmq_read, .9)
        except zmq.ZMQError:
            self.statuslbl.text="Status: Couldn't Connect to Server! Please restart application..."
            return

class ControlApp(App):
    def build(self):
        self.title = "YaskawaCoE Master"
        return controlWindow()

if __name__ == '__main__':
    Config.set('graphics', 'width',  690)
    Config.set('graphics', 'height', 600)
    Window.size = (690,600)
    ControlApp().run()
