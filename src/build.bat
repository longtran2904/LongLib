@echo off

set opts=-FC -GR- -EHa- -nologo -O2 -Oi -Z7
set code=%cd%
cd ..
pushd build
cl %opts% %code%\LongLib_Windows.c user32.lib gdi32.lib opengl32.lib Msimg32.lib
cl %opts% %code%\Meta\Tokenizer.c user32.lib gdi32.lib opengl32.lib Msimg32.lib

set opts=%opts% -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127
cl %opts%  %code%\LongApp_Windows.c -FeDemo.exe user32.lib gdi32.lib opengl32.lib Msimg32.lib
popd
