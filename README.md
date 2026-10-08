# qutIM Instant Messenger

qutIM is module-based multiprotocol instant messenger, based on Nokia Qt. All its functionality and features are implemented through separate plugins.

This fork keeps only XMPP and is ported to Qt 6. See [porting.md](./porting.md) for the plan.

## Links ##

~~Official site — http://qutim.org/~~ DEAD

~~Wiki — http://wiki.qutim.org/~~ DEAD

~~Official twitter — http://twitter.com/qutim~~ DEAD

If you want old releases — check this: https://github.com/euroelessar/qutim/releases/tag/archival_release

Main source repo — https://github.com/euroelessar/qutim

## How to build

Dependencies on Debian/Ubuntu:

    sudo apt install g++ cmake ninja-build pkgconf qt6-base-dev qt6-declarative-dev \
        qt6-svg-plugins libgsasl-dev qtkeychain-qt6-dev zlib1g-dev

`qt6-svg-plugins` is needed at runtime, without it most icons are not shown.

    git submodule update --init artwork translations
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build
    ./build/bin/qutim

The build directory has the installed layout, so qutim runs from it directly.
