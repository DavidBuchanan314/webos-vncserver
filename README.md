# webos-vncserver
An somewhat hacky VNC server for WebOS.

Requires root privileges.

# Usage

```
# ./gmvnc
```

You should then be able to connect via a VNC client of your choice, on port 5900.

Here it is, running on a "headless" TV motherboard, being accessed via a VNC mobile app.

![Demo](./img/demo.jpg?raw=true)

# Caveats

- This does not capture any hardware-accelerated video surfaces, only the UI layers.


# Compiling

To cross-compile for WebOS, you will need an NDK: https://github.com/webosbrew/meta-lg-webos-ndk

```sh
rm -rf build && mkdir build && cd build && cmake .. && cmake --build . --target gmvnc
```

You should have produced a `gmvnc` binary. Copy it over to your TV and run it as root!
