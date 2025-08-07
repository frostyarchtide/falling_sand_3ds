#!/bin/bash

PROJECT_NAME=falling_sand_3ds
ADDRESS=192.168.1.24

catnip -T 3ds \
&& cp build/main.release/compile_commands.json . \
&& 3dslink build/main.release/${PROJECT_NAME}.3dsx -a ${ADDRESS}
