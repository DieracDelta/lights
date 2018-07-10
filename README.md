# What is this? #

A work in progress openrc-style daemon and corresponding library for controlling the lights on the Alienware 15 R3 model laptop. (Fuck systemd. I won't be supporting them.)

You can also interface with the daemon in a higher level language such as python, as all it does is listen on a unix socket. However, I've opted to write my library in C, as most of *my* use cases involve incorporating the library into c programs.


# How to configure alienfxD: #

## dependencies ##

This daemon depends on lusb-1.0 (you do need to specify the version) and some sort of logging daemon implementation for the use of syslog (I'm using syslog-ng).

## installation ##

```bash
git clone http://diracdelta.me:3000/jrestivo/alienware_15_r3_light_daemon.git lightdaemon
cd lightdaemon/src/daemon
sudo make clean && sudo make && sudo make install
cd ../src/library
sudo make clean && sudo make && sudo make install
# now, enable the daemon enable as a service
sudo rc-service alienfxd start
sudo rc-service alienfxd enable
```

## usage ##

Communication with the daemon is essentially done over a socket. My implementation of commands have been done over a datagram socket via a specific protocol that I'll eventually get around to writing actual docs for. But for now, if you're curious, the code is pretty well documented. Have a read :)

Note that the daemon writes to 

# Progress #
  * [ ] Underlying protocol
    * [WIP] flashing between two different colors
    * [ ] other things you can do that I haven't figured out yet
    * [x] get state
    * [x] set rgb and freq
    * [x] toggle lights on/off
  * [ ] library portion
  * [x] daemon portion
  * [x] datagram socket communication + set a protocol between client and daemon
  * [ ] saving + persistent state between boots stored as json on a per-boot basis
  * [WIP] clean up code and makefiles
