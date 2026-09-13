@echo off

for /R %%i in (*.png) do (
	python3 "%~1" "%%i"
)