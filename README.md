RetroStream
===========

RetroStream is a C-based software that allows lossless video streaming from a 'publisher' to a 'subscriber'. It was created by Robin Southern (https://github.com/betajaen) and is licensed under the MIT license.

Description
-----------

The publisher can be a Raspberry Pi (or Linux compatible) device that takes video input from a USB Capture Device. The video is then sent using ZeroMQ as lossless frames. The subscriber then receives the video and presents it as-is as a Window on the screen. The user can then capture the Window using tools like OBS to allow the video to be streamed using platforms like Twitch.tv or Youtube.

In practice, there is about one frame of latency at 800x600 with RGB24 colour. 

This tool is due to a very unique situation that came about with video streaming on macOS.

Warning: The code is very experimental and various options are missing or hardcoded.

Dependencies
------------

The following dependencies are required to build and run RetroStream:

* czmq
* SDL3
* pthreads

Usage
-----

To use RetroStream, follow these steps:

Connect a USB Capture Device to your Raspberry Pi or Linux device.
Clone the RetroStream repository.
Use CMake to build the software.
Run the pub program on the publishing device.
Run the sub program on the receiving device.
License

RetroStream is licensed under the MIT License. See LICENSE for more information.

Credits
------

RetroStream was created by Robin Southern. You can find the original repository [here](https://github.com/betajaen/RetroStream).