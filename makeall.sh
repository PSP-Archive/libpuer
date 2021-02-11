#!/bin/sh
mkdir dist
cd libpuer
make && make clean
cd ..
cd 500DC8ex/source
make dist && make clean
cd ../..
cd autobranch/source
make dist && make clean
cd ../..
cd ctrlcheck/source
make -f Makefile_kernel dist && make -f Makefile_kernel clean
make dist && make clean
cd ../..
cd google2fa/source
make dist && make clean
cd ../..
cd popsdeco/source
make -f Makefile_rev1 dist && make -f Makefile_rev1 clean
make dist && make clean
cd ../..
cd ps1bioshelper/source
make dist && make clean
cd ../..
cd pspreboot/source
make dist && make clean
cd ../..
cd pspshutdown/source
make dist && make clean
cd ../..
cd puerforge/source
make dist && make clean
cd ../..
cd rcoredirector/source
make dist && make clean
cd ../..
cd usbenabler/source
make dist && make clean
cd ../..
cd wdayloader/source
make dist && make clean
cd ../..
#cd zenkaku_prx/source
#make dist && make clean
#cd ../..
