# Mercury

Mercury is FOSS designed to allow for decentralized, peer to peer streaming and chatting between small groups. Built in C++ using Qt and a custom networking library for real-time video and audio, it prioritizes speed, efficiency, and ease of use.

Read `docs/` for more information.

**Contributors**
- Alex Hunton
- Christopher Lee
- Kris Leungwattanakij
- Leonard Pan
- Kai Tinkess

### Getting Started
Prerequisites:
* Open-Source GPL Licensed Qt 6.8 Developer Tools (from [here](https://www.qt.io/download-qt-installer-oss))
* C++, CMake, Git

```bash
git clone ...
git submodule update --init --recursive
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=your qt installation
make
./mercury
```

An example path to pass into cmake could be: `cmake .. -DCMAKE_PREFIX_PATH=~/Qt/6.8.1/macos`.

### Screenshots
