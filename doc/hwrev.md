Hardware Revisions
==================

##  Rev. 1
- Problems:
	- USB D+ and D- lines are crossed → use flipped USB connector instead.
	- RF component footprints have solder resist on the pads → revise solder mask.
	- HMC602 footprint does not fit (pads are wider than package).


## Rev. 2
- Problems:
	- Pin numbering of button symbol is wrong (causes PGC to be shorted).
	- RF Temp requires a voltage divider towards MCU
	- Moved USB connector to PCB rear side (to swap D+ and D-).
- Changes:
	- Fixed solder resist on RF component footprints
	- Revised HMC602 footprint (pads were wider than package).
	- Fixed TMP112 footprint (pads were too wide).
	- Added more drill-holes close to RF connector.


## Rev. 3
Not produced yet.

- Changes:
	- Fixed button symbol and layout.
	- Added voltage divider between RF Temp output and MCU ADC input.
- Ideas:
	- Add "GND-island" around DCDC.
	- Add a GND clamp for easier hardware-debugging.

