**Driver for the SPH0645LM4H I2S MEMS Microphone**

[Product learn page on Adafruit](https://learn.adafruit.com/adafruit-i2s-mems-microphone-breakout/overview).


Installing as a stand-alone module
==============================================================================

    make
    sudo make install

To load the driver manually, run this as root:

    modprobe snd-i2s_rpi

You may also specify custom toolchains by using the `CROSS_COMPILE` flag:

    CROSS_COMPILE=/usr/local/bin/arm-eabi-


Installing as a DKMS module
==============================================================================

You can have even more fun with snd-i2s\_rpi by installing it as a DKMS module
which has the main advantage of being auto-compiled between kernel upgrades.

First, get dkms. On Raspbian this should be:

	sudo apt install dkms

Then copy the root of this repository to `/usr/src`:

	sudo cp -R . /usr/src/snd-i2s_rpi-0.0.3 (or whatever version number declared on dkms.conf is)
	sudo dkms add -m snd-i2s_rpi -v 0.0.3

Build and load the module:

	sudo dkms build -m snd-i2s_rpi -v 0.0.3
	sudo dkms install -m snd-i2s_rpi -v 0.0.3

Now you have a proper dkms module that will work for a long time... hopefully.


Testing the I2S microphone driver
==============================================================================

```bash

# For this to work, turn on I2S support by adding
#   dtparam=i2s=on to /boot/config.txt

$ sudo sed -i -e 's/#dtparam=i2s=on/dtparam=i2s=on/g' /boot/config.txt

# Make sure sound support is enabled in the kernel by adding
#   snd-bcm2835 to /etc/modules

$ sudo sh -c 'echo snd-bcm2835 >> /etc/modules'

# Reboot if you want to test the module before loading it automatically

$ sudo reboot

# Load the module for Raspberry Pi 1 B/A/A+, Zero, Zero W
$ sudo modprobe snd-i2s_rpi

# Add argument rpi_platform_generation=1 for Raspberry Pi 2 or 3
# sudo modprobe snd-i2s_rpi rpi_platform_generation=1

# see if it works

$ dmesg | grep i2s

# it should say 20203000.i2s (or 3f203000.i2s) mapping OK

# [    3.519017] snd_i2s_rpi: loading out-of-tree module taints kernel.
# [    3.519881] snd-i2s_rpi: Version 0.0.3
# [    3.519889] snd-i2s_rpi: Setting platform to 20203000.i2s
# [    9.507142] asoc-simple-card asoc-simple-card.0: snd-soc-dummy-dai <-> 20203000.i2s mapping ok

# Running arecord -l should list your new capture device

$ arecord -l

# **** List of CAPTURE Hardware Devices ****
# card 1: sndrpii2scard [snd_rpi_i2s_card], device 0: simple-card_codec_link snd-soc-dummy-dai-0 [simple-card_codec_link snd-soc-dummy-dai-0]
#   Subdevices: 0/1
#   Subdevice #0: subdevice #0

# If you want it to load automatically at startup then
# add snd-i2s_rpi to /etc/modules

$ sudo sh -c 'echo snd-i2s_rpi >> /etc/modules'

# If you have a Raspberry Pi 2 or 3, you need to do this:
# create a file called /etc/modprobe.d/snd-i2s_rpi.conf
# and add this line:
# options snd-i2s_rpi rpi_platform_generation=1

$ sudo touch /etc/modprobe.d/snd-i2s_rpi.conf
$ sudo sh -c 'echo "options snd-i2s_rpi rpi_platform_generation=1" >> /etc/modprobe.d/snd-i2s_rpi.conf'

# Test recording with arecord

$ arecord -D plughw:1 -c1 -r 48000 -f S32_LE -t wav -v recording.wav

# If you want to change the volume then create a configuration file for ALSA
# in your home folder and add a software volume device.

$ cat >> .asoundrc << 'EOF'
pcm.micsw {
  type softvol
  slave.pcm "michw"
  control {
    name "micctrl"
    card sndrpii2scard
  }
  min_dB -3.0
  max_dB 20.0
}

pcm.michw {
  type hw
  card sndrpii2scard
  channels 2
  format S32_LE
}
EOF

# You might have to use the soft volume device once before the volume slider
# shows up in alsamixer.

$ arecord -D micsw -c2 -r 48000 -f S32_LE -t wav -V mono -v recording.wav

# If you want to use the microphone using pulseaudio then
# add the following to /etc/pulse/default.pa

load-module module-alsa-source device=micsw source_name=microphone
set-default-source microphone

# And add the lines below to /etc/pulse/daemon.conf

default-sample-format = s32le
default-sample-rate = 48000

```


