#!/bin/sh

# Create Backscatter file
octave writeBscFile.m

# Create Phantom
./createPhantom simCreatePhantomInput.txt

# Create rf data
./rfDataProgram rfDataInputTemplate.txt

# Convert the binary data to image
octave binary2matrix.m
