[![Build Status](https://travis-ci.org/BenBoZ/ultrasound-simulation.svg?branch=master)](https://travis-ci.org/BenBoZ/ultrasound-simulation)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/4610/badge.svg)](https://scan.coverity.com/projects/4610")

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

License
=======

Note that this code is provided under a modified MIT license. In addition to the MIT
license terms, the authors ask you to cite the following paper in all published
work that uses this or derivatives of this work.

    Yadong Li, James A. Zagzebski, ``A Frequency Domain Model for Generating B-Mode
    Images with Array Transducers,'' IEEE Transactions on Ultrasonics,
    Ferroelectrics, and Frequency Control, vol. 46, no. 3, May 1999.
