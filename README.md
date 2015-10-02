facebook-tunnel
===============

[![Join the chat at https://gitter.im/matiasinsaurralde/facebook-tunnel](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/matiasinsaurralde/facebook-tunnel?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

The idea of this project is to tunnel Internet traffic through Facebook chat (packets are sent as base64), the main component is tuntap and also the Google's Gumbo parser which does the interaction with Facebook (login, send/receive messages, etc.).


Why?
----
I'm from Paraguay, South America. Our country was chosen for the Internet.org campaign: "free Internet access in developing countries". TIGO, one of the biggest telcos here, negotiated a deal so you can have free Facebook access through them.
We all know that the Internet is about accessing many places, etc. So I feel this campaign as a serious limitation.

Requirements
------------

* Gumbo parser
* libcurl
* tuntap (for linux)
* gcc/g++

Building
------------
```
$ mkdir build; cd build
$ cmake ..
$ make
$ ./facebook-tunnel
```


TODO
----

* Finish the client.
* Obfuscation and/or crypto.
* Support other platforms (kennym is working on Mac support, Android would be nice).

I'm looking for collaborators, so if you have something interesting, send a pull request.


You can also shoot me an e-mail: matias@insaurral.de / matias@heinetwork.com

Twitter: [@matias_baruch](https://twitter.com/matias_baruch)
