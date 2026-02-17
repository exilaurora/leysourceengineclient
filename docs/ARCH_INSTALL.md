Installation and running notes for Arch Linux

These steps describe building and running `leysourceengineclient` on Arch Linux (the binary is 32-bit and requires the Steam 32-bit runtime).

1) Enable multilib
- Edit `/etc/pacman.conf`, uncomment the `[multilib]` section and its `Include = /etc/pacman.d/mirrorlist` line.
- Update package lists:

```sh
sudo pacman -Syu
```

2) Install toolchain and common 32-bit libraries (adjust to your needs):

```sh
sudo pacman -S --needed base-devel git
sudo pacman -S --needed lib32-mesa lib32-libxrandr lib32-libxi lib32-libxrender lib32-libxtst lib32-openal lib32-pulse lib32-libx11

sudo pacman -S lib32-libxrandr lib32-libxi lib32-libxrender lib32-libxtst lib32-openal lib32-pulseaudio
```

3) Install Steam (provides `steamclient.so` and runtime libs):

```sh
sudo pacman -S steam
```

4) Install `premake5` (used to generate makefiles). On Arch it may be available in community or via AUR:

```sh
# if community package exists
sudo pacman -S premake
# or via AUR (example helper):
yay -S premake5
```

5) Build the project (from repository root):

```sh
premake5 gmake
make config=release
```

6) Run the binary using the Steam 32-bit runtime (adjust path if needed):

```sh
export LD_LIBRARY_PATH="$HOME/.local/share/Steam/ubuntu12_32:$LD_LIBRARY_PATH"
/path/to/repo/build/release/leysourceengineclient -serverip <ip> -serverport <port>
```

Alternative: copy the 32-bit `steamclient.so` next to the binary and run with that folder first in `LD_LIBRARY_PATH`:

```sh
cp ~/.local/share/Steam/ubuntu12_32/steamclient.so /path/to/repo/build/release/
LD_LIBRARY_PATH=/path/to/repo/build/release:$LD_LIBRARY_PATH /path/to/repo/build/release/leysourceengineclient -serverip <ip> -serverport <port>
```

Troubleshooting tips
- Check for missing shared libraries:

```sh
ldd ~/.local/share/Steam/ubuntu12_32/steamclient.so | grep "not found"
```

- Use loader debug to inspect `dlopen` attempts:

```sh
LD_DEBUG=libs LD_LIBRARY_PATH="$HOME/.local/share/Steam/ubuntu12_32:$LD_LIBRARY_PATH" /path/to/repo/build/release/leysourceengineclient 2>&1 | sed -n '1,200p'
```

- Verify binary architecture matches Steam runtime:

```sh
file /path/to/repo/build/release/leysourceengineclient
```

If you want, I can add a small helper script to set `LD_LIBRARY_PATH` and run the client automatically.
