#import json
import struct
import zmq
from kivy.app import App
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
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect("tcp://localhost:5555")
        Clock.schedule_interval(self._zmq_read, .9)
        return super().__init__(**kwargs)

    def _zmq_read(self, dt):
        try:
            self.socket.send_string("Request")
            data = self.socket.recv()
            #self.possts.text=''.join('{:02x}'.format(x) for x in data[4:8])
            self.postar.text=str(int.from_bytes(data[10:14],byteorder='little'))
            self.possts.text=str(int.from_bytes(data[4:8],byteorder='little'))
            self.statusword.text=''.join('{:02x}'.format(x) for x in data[2:4])
            self.controlword.text=''.join('{:02x}'.format(x) for x in data[8:10])
        except zmq.ZMQError:
            pass

class ControlApp(App):
    def build(self):
        return controlWindow()

if __name__ == '__main__':
    ControlApp().run()
