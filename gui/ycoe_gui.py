#import json
import struct
import zmq
from kivy.app import App
from kivy.config import Config
from kivy.core.window import Window
from kivy.uix.button import Button
from kivy.uix.floatlayout import FloatLayout
from kivy.clock import Clock

class RelMoveBtn(Button):
    def move_relative(self, distance):
        ctrlwindow=self.parent.parent.parent
        cmdmsg='{"type":"relative","distance":"%s"}' % ( distance)
        ctrlwindow.socket.send(cmdmsg)
        print(cmdmsg)

class RelGoBtn(Button):
    def move_relative(self, distance):
        ctrlwindow=self.parent.parent.parent
        cmdmsg='{"type":"relative","distance":"%s"}' % ( distance)
        ctrlwindow.socket.send(cmdmsg)
        print(cmdmsg)

class AbsMoveBtn(Button):
    def move_absolute(self, distance):
        ctrlwindow=self.parent.parent.parent
        #cmdmsg=json.dumps({"type":"absolute","distance":distance})
        #print([distance])
        ctrlwindow.socket.send(distance)
        message = ctrlwindow.socket.recv()
        #print("Received reply %s [ %s ]" % (cmdmsg, message))

class CmdPosBtn(Button):
    def move_cmdpos(self, strposition):
        ctrlwindow=self.parent.parent.parent
        distance=int(strposition)
        if distance>4294967295:
            distance=4294967295
        elif distance<0:
            distance=0
        #cmdmsg=json.dumps({"type":"absolute","distance":distance})
        #print([distance, distance.to_bytes(4, byteorder='little'), struct.pack(distance])
        #print(struct.pack('<BI',3,distance))
        ctrlwindow.socket.send(struct.pack('<BI',3,distance))
        message = ctrlwindow.socket.recv()
        #print("Received reply %s [ %s ]" % (cmdmsg, message))

class RegReadBtn(Button):
    def readreg(self, slaveaddr, regaddr):
        ctrlwindow=self.parent.parent.parent
        islaveaddr=int(slaveaddr)
        iregaddr=int(regaddr,16)
        if islaveaddr > 0xFFFF:
            islaveaddr = 0xFFFF
        if iregaddr > 0xFFFF:
            iregaddr = 0xFFFF
        ctrlwindow.socket.send(struct.pack('<BII',6,islaveaddr,iregaddr))
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
        ctrlwindow.socket.send(struct.pack('<BIII',9,islaveaddr,iindex,isubindex))
        message = ctrlwindow.socket.recv()

class AbsGoBtn(Button):
    def move_absolute(self, distance):
        ctrlwindow=self.get_parent_window
        cmdmsg=json.dumps({"type":"absolute","distance":distance})
        ctrlwindow.socket.send(cmdmsg)
        print(cmdmsg)


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
            self.statuslbl.text="Status:"
            #self.possts.text=''.join('{:02x}'.format(x) for x in data[4:8])
            self.postar.text=str(int.from_bytes(data[10:14],byteorder='little'))
            self.possts.text=str(int.from_bytes(data[4:8],byteorder='little'))
            self.statusword.text=''.join('{:02x}'.format(x) for x in reversed(data[2:4]))
            #self.statusword.text=str(int.from_bytes(data[2:4],byteorder='little'))
            #self.statusword.text=str(struct.unpack("<H",data[2:4]))
            self.controlword.text=''.join('{:02x}'.format(x) for x in reversed(data[8:10]))
            #self.controlword.text=str(int.from_bytes(data[8:10],byteorder='little'))
            Clock.schedule_once(self._zmq_read, .9)
        except zmq.ZMQError:
            self.statuslbl.text="Status: Couldn't Connect to Server! Please restart application..."
            return

class ControlApp(App):
    def build(self):
        self.title = "YaskawaCoE Master"
        return controlWindow()

if __name__ == '__main__':
    Config.set('graphics', 'width',  900)
    Config.set('graphics', 'height', 600)
    ControlApp().run()
