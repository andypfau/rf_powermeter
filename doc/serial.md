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
	
	Cont  Avg x16  1000 MHz

Valid commands:

- `t`: trigger a single measurement
- `c`: continuous trigger mode
- `a`: enter number of averages (must be a power of 2, in the range 1..512)
- `f`: enter frequency in MHz (for compensation data; must be in the range 10..8000)
- `Esc`: re-draw the screen
- `h`: display help


# Remote Mode

Send a `\0` character (i.e. ASCII code zero) to enter remote mode. Valid commands:

- `t`: trigger a measurement
	- Ehe response will be sent as a signed floating-point string, followed by `\n`.
	- E.g. `t` → `-30.205\n`.
- `a<int>ņ`: set number of averages
	- Must be a power of two.
	- E.g. `a32\n` to set 32 averages.
- `d`: diagnostics
	- Returns three semicolon-separated list of three values: the measured voltage of 5V0USB (USB bus voltage) in volts, the measured voltage of 5V0A (analog supply) in volts, and the tmeperature sensor reading on °C
	- E.g. `d` → `4.999;5.010;32.105\n`.
- `\0`: enter remote mode
