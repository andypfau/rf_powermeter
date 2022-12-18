import serial
import logging
from dataclasses import dataclass


def is_power_of_2(i: int) -> bool:
    return (i & (i-1)) == 0


class Powermeter:

    @dataclass
    class Diag:
        UsbV: float
        Analog5V: float
        TempC: float

        def __str__(self) -> str:
            return f'5V0USB = {self.UsbV:+.3f} V, 5V0A: {self.Analog5V:+.3f} V, Temperature: {self.TempC:+.2f} °C'


    def __init__(self, port: str, offline: bool = False):
        self.offline = offline
        self._connect(port)
    

    def _connect(self, port: str):
        if self.offline:
            return
        self.ser = serial.Serial(port=port, timeout=3.0)
        self.ser.write([0]) # start communication


    def _send(self, cmd: str):
        self.ser.write(cmd.encode('ASCII'))

    
    def _receive(self) -> str:
        buf = self.ser.read_until(b'\n')
        return buf.decode('ASCII').strip('\n')
    

    def reset(self):
        self.set_averaging(16)
        self.set_frequency(10e6)
        self.enable_cal(True)

    
    def set_averaging(self, n: int):
        if n<1 or n>512 or not is_power_of_2(n):
            raise ValueError(f'Averaging must be a power of 2 in the range 1..512')
        self._send(f'a{n}\n')

    
    def set_frequency(self, hz: float):
        mhz = int(round(hz/1e6))
        if mhz<10 or mhz>6000:
            raise ValueError(f'Frequency must be 10..6000 MHz')
        self._send(f'f{hz}\n')

    
    def enable_cal(self, enable: bool = True):
        self._send(f'm{"1" if enable else "0"}\n')

    
    def measure(self) -> float:
        
        if self.offline:
            return -99
        
        try:
            self._send('t')
            buf = self._receive()
            db = float(buf)
            return db

        except Exception as ex:
            raise RuntimeError(f'Unable to query diag ({ex})')

    
    def diag(self) -> "Powermeter.Diag":
        
        if self.offline:
            v_usb = 5
            v_a = 5
            temp = 20
        
        else:
            try:
                self._send('d')
                buf = self._receive()
                values = [float(part) for part in buf.split(';')]
                [v_usb, v_a, temp] = values

            except Exception as ex:
                raise RuntimeError(f'Unable to query diag ({ex})')
            
        return Powermeter.Diag(v_usb, v_a, temp)

    
    def write_to_eeprom(self, address: int, word16b: int):
        assert (address & 0xFFFF) == address, '<address> must be a 16 bit value'
        assert (word16b & 0xFFFF) == word16b, '<word16b> must be a 16 bit value'
        self._send(f'w{address:04X}{word16b:04X}\n')
