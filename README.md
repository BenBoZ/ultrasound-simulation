[![Build Status](https://travis-ci.org/BenBoZ/ultrasound-simulation.svg?branch=master)](https://travis-ci.org/BenBoZ/ultrasound-simulation)

This program has 3 parts.  The first part creates a phantom file, which is
mostly a random collection of uniformly distributed scatterers.  Each scatterer
is a spherical scatterer that reflects sound from a simulated ultrasound transducer.

The second part is used to displace scatterers, and create a second deformed phantom.  
This is useful for simulating elastographic imaging.

The third part of the program simulates ultrasound image formation in the frequency domain.

Compiling
=========

    mkdir -p bld
    cd bld
    cmake ..
    make

Running
=======
It is not doing a lot now, but it is at least running something

From within compiled bld folder (see steps above)

    ./runAll.sh

