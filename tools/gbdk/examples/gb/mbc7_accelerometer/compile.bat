REM Automatically generated from Makefile

..\..\..\bin\lcc -Wf-MMD -Wf-Wp-MP  -msm83:gb -c -o obj\gb\main.o src\main.c
..\..\..\bin\lcc -Wf-MMD -Wf-Wp-MP  -msm83:gb -c -o obj\gb\mbc7_accelerometer.o src\mbc7_accelerometer.c
..\..\..\bin\lcc -Wf-MMD -Wf-Wp-MP  -msm83:gb -c -o obj\gb\vsync_no_halt.o src\vsync_no_halt.c
..\..\..\bin\lcc -Wl-yt0x22   -Wl-j  -Wf-MMD -Wf-Wp-MP  -Wf-MMD -Wf-Wp-MP  -msm83:gb -o build\gb\mbc7_accelerometer.gb obj\gb\main.o obj\gb\mbc7_accelerometer.o obj\gb\vsync_no_halt.o 
