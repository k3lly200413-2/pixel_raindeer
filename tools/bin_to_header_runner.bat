@echo off
setlocal enabledelayedexpansion
set FILES=
for /R %%i in (*.bin) do (
    set FILES=!FILES! "%%i"
)
python3 "%~1" !FILES! --width 20 --height 20 --name deer --out DeerSprite.h
