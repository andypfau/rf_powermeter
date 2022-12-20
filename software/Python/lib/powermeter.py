import serial
from dataclasses import dataclass


MEM_MAX_ENTRIES = 1638

MEM_ADDR_CAL_DATA_COUNT        = 0x0000
MEM_ADDR_CAL_FREQ_DATA_START   = 0x4000
MEM_ADDR_CAL_SLOPE_DATA_START  = 0x8000
MEM_ADDR_CAL_OFFSET_DATA_START = 0xC000


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
        if len(buf) == 0:
            raise RuntimeError('No response after timeout')
        return buf.decode('ASCII').strip('\n')

    
    def _check_for_errors(self):
        
        if self.offline:
            return
        
        try:
            self._send('e')
            buf = self._receive()
            error = int(buf)
            if error != 0:
                raise RuntimeError(f'Device reported error code {error}')

        except Exception as ex:
            raise RuntimeError(f'Unable to query error code ({ex})')
    

    def reset(self):
        self.set_averaging(16)
        self.set_frequency(10e6)
        self.enable_cal(True)

    
    def set_averaging(self, n: int):
        if n<1 or n>512 or not is_power_of_2(n):
            raise ValueError(f'Averaging must be a power of 2 in the range 1..512')
        self._send(f'a{n}\n')
        self._check_for_errors()

    
    def set_frequency(self, hz: float):
        mhz = int(round(hz/1e6))
        if mhz<10 or mhz>6000:
            raise ValueError(f'Frequency must be 10..6000 MHz')
        self._send(f'f{mhz}\n')
        self._check_for_errors()

    
    def enable_cal(self, enable: bool = True):
        self._send(f'm{"1" if enable else "0"}\n')
        self._check_for_errors()

    
    def measure(self) -> float:
        
        if self.offline:
            return -99
        
        try:
            self._send('t')
            buf = self._receive()
            db = float(buf)

        except Exception as ex:
            raise RuntimeError(f'Unable to query reading ({ex})')
        
        self._check_for_errors()
        return db

    
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
        
        self._check_for_errors()
            
        return Powermeter.Diag(v_usb, v_a, temp)

    
    def _write_to_eeprom(self, address: int, data_16b: int):
        assert (address & 0xFFFF) == address, '<address> must be a 16 bit value'
        assert (data_16b & 0xFFFF) == data_16b, '<data_16b> must be a 16 bit value'
        self._send(f'w{address:04X}{data_16b:04X}\n')
        #self._check_for_errors()

    
    def _prepare_cal_data(self, frequencies_hz: "list[float]", errors_db: "list[float]") -> "tuple[list[float],list[float],list[float]]":
        
        if len(frequencies_hz) != len(errors_db):
            raise ValueError(f'Number of entries must be the same for both lists')
        if not (2 <= len(frequencies_hz) <= MEM_MAX_ENTRIES-1):
            raise ValueError(f'Number of entries must be 2..{MEM_MAX_ENTRIES}')

        freqs, slopes, offsets = [], [], []
        for i in range(1, len(frequencies_hz)):

            if frequencies_hz[i-1] >= frequencies_hz[i]:
                raise ValueError(f'Frequency list must be monotonic (check index {i})')

            f_mhz = int(round(frequencies_hz[i]/1e6))
            slope = int(round((errors_db[i] - errors_db[i-1]) / (frequencies_hz[i] - frequencies_hz[i-1])))
            offset = int(round(errors_db[i] - round(slope) * slope))

            freqs.append(f_mhz)
            slopes.append(slope)
            offsets.append(offset)    
        
        return freqs, slopes, offsets


    def write_cal_cata(self, frequencies_hz: "list[float]", errors_db: "list[float]"):

        freqs, slopes, offsets = self._prepare_cal_data(frequencies_hz, errors_db)
        n = len(freqs)
        
        assert 0 < n <= MEM_MAX_ENTRIES

        self._write_to_eeprom(MEM_ADDR_CAL_DATA_COUNT, n)
        for i,freq in enumerate(freqs):
            self._write_to_eeprom(MEM_ADDR_CAL_FREQ_DATA_START + i*2, freq)
        for i,slope in enumerate(slopes):
            self._write_to_eeprom(MEM_ADDR_CAL_SLOPE_DATA_START + i*4 + 0, (slope>>16)&0xFFFF)
            self._write_to_eeprom(MEM_ADDR_CAL_SLOPE_DATA_START + i*4 + 2, (slope>> 0)&0xFFFF)
        for i,offset in enumerate(offsets):
            self._write_to_eeprom(MEM_ADDR_CAL_OFFSET_DATA_START + i*4 + 0, (offset>>16)&0xFFFF)
            self._write_to_eeprom(MEM_ADDR_CAL_OFFSET_DATA_START + i*4 + 2, (offset>> 0)&0xFFFF)
