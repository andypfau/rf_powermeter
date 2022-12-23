Serial Interface
================

# Getting Started

1. Connect the Powermeter via USB to your PC
	- both under Windows and under Linux, a virtual serial port should be detected without drivers needed
2. Connect to the virtual serial port e.g. with *PuTTY*, or e.g. with the `screen` command


# UI Mode

Press the `Esc` key to enter UI mode.

	-35.103 dB
	
	          v
	| ' | ' | ' | ' | ' | ' | ' |
	
	Cnt  Avg 16  3000 MHz


Valid commands:

- `a`: enter number of averages (must be a power of 2, in the range 1..512)
- `c`: continuous trigger mode
- `d`: display diagnostic data
- `t`: trigger a single measurement
- `f`: enter frequency in MHz (for compensation data; must be in the range 10..8000)
- `h`: display help
- `l`: turn compensation on/off
- `Esc`: re-draw the screen


# Remote Mode

Send a `\0` character (i.e. ASCII code zero) to enter remote mode. All commands and responses are terminated by `\n` (i.e. ASCII code 10) Valid commands:

- `a<int>\n`: set number of averages
	- Must be a power of two
	- E.g. `a32\n` to set 32 averages
- `d\n`: diagnostics
	- Returns three semicolon-separated list of three values: the measured voltage of 5V0USB (USB bus voltage) in volts, the measured voltage of 5V0A (analog supply) in volts, and the tmeperature sensor reading on °C
	- E.g. `d\n` → `4.999;5.010;32.105\n`
- `e\n`: get last error
	- Returns an error code that is zero in the case of no error.
	- E.g. `e\n` → `0\n`
- `f<int>\n`: set frequency, in MHz
	- Must be in the range 10..8000
	- E.g. `f1100\n` to set to 1.1 GHz
- `l<int>\n`: enable/disable compensation
	- Must be 1 (enable) or 0 (disable)
	- E.g. `l1\n` to enable compensation
- `m<r|w><hex><hex>\n`: access EEPROM
	- Read from memory: send `r`, followed by exactly four hexadecimal digits (address); the response will be exactly four hexadecimal digits (data)
	- Write to memory: send `w`, followed by exactly four hexadecimal digits (address), followed by exactly four hexadecimal digits (data)
	- E.g. `mw00010002\n` to write 0x0002 to address 0x0001
	- E.g. `mr0001\n` to read from address 0x0001 → e.g. `0002\n`
- `t\n`: trigger a measurement
	- Ehe response will be sent as a signed floating-point string, followed by `\n`
	- E.g. `t\n` → `-30.205\n`
- `\0`: enter remote mode
