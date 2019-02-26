# cpp-calculator

Hey, so this is my custom calculator that I wrote a few months ago.

If you want to build this, you'll need CMake and Emscripten (and a modification to [CMakeLists.txt](https://github.com/LoganDark/cpp-calculator/blob/master/CMakeLists.txt#L11)). If you want to work on this, you'll need CLion.

## How to build it

Run these commands:

```
git clone https://github.com/LoganDark/cpp-calculator
cd cpp-calculator/
git submodule init
git submodule update
cd boilerplate
git submodule init
git submodule update
cd ..
```

Edit `../CMakeLists.txt` to point to the correct `em++`, then run:

```
mkdir cmake-build-debug
cd cmake-build-debug/
cmake ..
make
```

There you go. The `calculator` binary is in the current directory and the emscripten output is in the `../emscripten` folder.

## How to work on it / contribute to it

If you don't want to contribute to it, sure, just edit whatever files you need and then run.
But if you want to contribute to it, there's a very specific process I'd like you to follow to ensure that the code style stays consistent.

First, you'll need CLion. No other IDE will do, because the style I use is for CLion's formatter.
Besides, CLion is one of the best and it's free for students.
Open up CLion and load the `cpp-calculator` directory as a project.

Make sure to import all the code styles in [`intellij_styles`](https://github.com/LoganDark/cpp-calculator/tree/master/intellij_styles).
Basically, the filename denotes where to go in the `Editor > Code Style` preferences.
Go there, select the Project scheme, click the little 3 dots next to the dropdown, and import the XML for every XML in there.
This is important.

Now that you have the code styles, everything should be set. Just make sure to mash the code formatter every few lines and make things pretty. :)

You can use the Run button if you want, but the line editing doesn't work there. I mean line editing of any kind.
Come to think of it, I don't think typing works much at all in there. CLion's retarded code editor terminal emulator sucks.
I wish CLion had a setting like PyCharm that let you use a proper terminal emulator for Run.
Apparently they thought Python needed a terminal emulator but C++ didn't.

Run also builds the Emscripten stuff too. I'd recommend just using Build (usually F9 by default) and running the binary manually.
You can also use the debugger, but unfortunately due to my limited knowledge of CMake it also builds the Emscripten version every time.
The `emscripten` directory contains everything you need to run the calculator in a web browser after building it at least once.

Make sure not to commit your modified `CMakeLists.txt`!

## Known Issues
One issue I've been facing for a while is that the `:prec` command doesn't work in the CLI.
No matter what, it seems like everything is fine, the value is written and everything but it just *disappears* randomly once the function ends.
Some help with this would be greatly appreciated.

Also, prediction isn't a thing. `x = 5; x + 5` in the web interface doesn't work if `x` isn't already defined.
This could be solved by sending the whole stream of commands to the calculator and adding onto the predictive functionality.
Possibly some kind of context that's per-prediction?
