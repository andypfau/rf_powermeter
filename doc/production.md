Production
==========

## Tools and Material

- PCB and stencil: [Aisler 4-layer](https://aisler.net/)
- Stencil registration tool: [Eurocrictuis ec manual tool](https://www.eurocircuits.de/ec-registration-system-details-platine-schablone/)
- Tools for pick&place: VAC-12000 vacuum pick&place, [SWIFT S41-20 Binocular Stereo Microscope](https://us.swiftmicroscopes.com/collections/stereo-microscope/products/swift-s41-20-binocular-stereo-microscope-with-10x-20x-and-1w-led-illumination)
- Solder paste: [Chipquik NC191SNL50T5](https://www.chipquik.com/store/product_info.php?products_id=473023) (Sn96.5/Ag3.0/Cu0.5, 217..220 °C melting point, no-clean flux, T5 / 15-25 microns)
- Vapor phase liquid: [Galden HS240](https://www.solvay.com/en/brands/galden-pfpe) (240 °C boiling point)


## Notes on Components

The key component, the [HMC602LP4E](https://www.analog.com/en/products/hmc602.html) RF power sensor, was [discontinued](https://www.analog.com/media/en/PCN/ADI_PDN_22_0023_Rev_B_Form.pdf) shortly after I started the project.

There is no pin-compatible replacement, but ADI recommends the [AD8318](https://www.analog.com/en/products/ad8318.html) as a replacement. At a first glance, it looks like it could be used with only small PCB modifications. However, it has a different pitch (0.65 mm vs. 0.5 mm), so the pad geometry may have to be adjusted.
