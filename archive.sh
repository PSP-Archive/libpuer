rm -f libpuer_dist.7z libpuer_src.7z
rm -f libpuer/include/psp.h.gch libpuer/include/kernelsdk.h.gch
cd dist
7z a -r0 -mx=9 -xr0!*/.svn/* -xr0!*/.svn ../libpuer_dist.7z *
cd ..
7z a -r0 -mx=9 -xr0!*/.svn/* -xr0!*/.svn libpuer_src.7z \
archive.sh gengch.sh genimport.pl genstub.pl makeall.sh libpuer \
500DC8ex autobranch ctrlcheck google2fa popsdeco pspreboot pspshutdown puerforge \
rcoredirector usbenabler wdayloader ps1bioshelper
#zenkaku_prx
