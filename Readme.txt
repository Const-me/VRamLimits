In many places on the Internets, e.g. https://www.gamasutra.com/view/feature/3602/sponsored_feature_ram_vram_and_.php, there’s a statement that only 64-bit apps can access more than 4GB VRAM.

That’s simply not true anymore.

On my windows 10 PC, 32-bit build prints following:

Reported VRAM: 3221225472 bytes, 3.000000 GB
Usable VRAM: 15099494400 bytes, 14.062500 GB

64 bit build:

Reported VRAM: 11672748032 bytes, 10.871094 GB
Usable VRAM: 15116271616 bytes, 14.078125 GB

As you see, a 32-bit app can allocate much more than 4GB of textures just fine.

However, the DXGI_ADAPTER_DESC::DedicatedVideoMemory field is clearly wrong in 32 bit builds.