# Debugger Setup (STM32F429 Discovery, Linux/WSL)

Concise steps to get on-chip debugging working on a fresh Linux/WSL machine after
cloning this project. Covers both **ST-Link** and **J-Link** (reflashed ST-Link-OB).

The `launch.json` configs reference the toolchain by **direct absolute paths** into
the STM32 VS Code extension's installed bundles (no standalone STM32CubeCLT and no
symlink tree required). The version numbers in those paths must match what the
extension has installed.

---

## 1. Fix a stale CMake cache (if the project was copied from another machine)

CMake stores absolute paths in its cache. If configure fails with
"CMakeCache.txt directory is different..." delete the stale cache and reconfigure:

```bash
rm -rf build/Debug          # or the affected build dir
rm -f build/CMakeCache.txt build/cmake_install.cmake
```

Then re-run **CMake: Configure**.

---

## 2. Locate the extension toolchain bundles

The STM32 VS Code extension installs its toolchain under
`~/.local/share/stm32cube/bundles`. Note the exact version folder names — they are
used in the paths in step 4:

```bash
ls -d ~/.local/share/stm32cube/bundles/stlink-gdbserver/*/bin/ST-LINK_gdbserver
ls -d ~/.local/share/stm32cube/bundles/gnu-tools-for-stm32/*/bin/arm-none-eabi-gdb
ls -d ~/.local/share/stm32cube/bundles/programmer/*/bin
```

Use the `gnu-tools-for-stm32` version that matches the build toolchain
(this project uses `13.3.1+st.9`).

---

## 3. Download the SVD file (peripheral register viewer)

The bundles do not ship an SVD, so fetch one to a stable location:

```bash
mkdir -p ~/.local/share/stm32cube/svd
curl -fsSL -o ~/.local/share/stm32cube/svd/STM32F429.svd \
  "https://raw.githubusercontent.com/cmsis-svd/cmsis-svd-data/main/data/STMicro/STM32F429.svd"
```

---

## 4. Set the direct paths in `launch.json`

Each cortex-debug config points straight at the installed binaries. Using the
versions from step 2, the relevant fields look like this (replace `<user>` and the
version folders with your own):

```jsonc
"svdFile":             "/home/<user>/.local/share/stm32cube/svd/STM32F429.svd",
"serverpath":          "/home/<user>/.local/share/stm32cube/bundles/stlink-gdbserver/7.13.0+st.3/bin/ST-LINK_gdbserver",
"stlinkPath":          "/home/<user>/.local/share/stm32cube/bundles/stlink-gdbserver/7.13.0+st.3/bin/ST-LINK_gdbserver",
"stm32cubeprogrammer": "/home/<user>/.local/share/stm32cube/bundles/programmer/2.22.0+st.1/bin",
"armToolchainPath":    "/home/<user>/.local/share/stm32cube/bundles/gnu-tools-for-stm32/13.3.1+st.9/bin",
"gdbPath":             "/home/<user>/.local/share/stm32cube/bundles/gnu-tools-for-stm32/13.3.1+st.9/bin/arm-none-eabi-gdb",
```

Verify every path exists:

```bash
for p in \
  ~/.local/share/stm32cube/svd/STM32F429.svd \
  ~/.local/share/stm32cube/bundles/stlink-gdbserver/7.13.0+st.3/bin/ST-LINK_gdbserver \
  ~/.local/share/stm32cube/bundles/programmer/2.22.0+st.1/bin \
  ~/.local/share/stm32cube/bundles/gnu-tools-for-stm32/13.3.1+st.9/bin/arm-none-eabi-gdb; do
  [ -e "$p" ] && echo "OK   $p" || echo "MISS $p"
done
```

> If you install the real standalone STM32CubeCLT instead, point these fields at
> that install (e.g. `/opt/st/stm32cubeclt_1.20.0/...`).

---

## 5a. Debug with ST-Link (default Discovery firmware)

No extra install needed once steps 3–4 are done.
Run and Debug → **Build & Debug Microcontroller - ST-Link**.

---

## 5b. Debug with J-Link (Discovery reflashed to J-Link-OB)

Install the SEGGER J-Link software pack (creates `/usr/bin/JLinkGDBServer`):

```bash
# Download (accepts SEGGER license via POST), x86_64:
curl -sL -X POST \
  -d "accept_license_agreement=accepted&non_emb_ctr=confirmed&submit=Download+software" \
  "https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb" \
  -o JLink_Linux_x86_64.deb

sudo apt install -y ./JLink_Linux_x86_64.deb
rm -f JLink_Linux_x86_64.deb        # don't commit the 51 MB installer

/usr/bin/JLinkGDBServer -version     # confirm it runs
```

The J-Link configs already use `serverpath: "/usr/bin/JLinkGDBServer"`.
Run and Debug → the **J-Link** configuration (device `STM32F429ZITx`, SWD).

---

## 6. WSL only: forward the probe over USB

The ST-Link / J-Link is a USB device. From an **admin PowerShell on Windows**:

```powershell
usbipd list                       # find the probe's BUSID
usbipd bind   --busid <BUSID>     # once per probe
usbipd attach --wsl --busid <BUSID>
```

Then in WSL confirm it is visible:

```bash
lsusb        # should list STMicroelectronics ST-LINK or SEGGER J-Link
```

---

## Quick reference

| Item | Path |
| --- | --- |
| ST-Link gdbserver | `~/.local/share/stm32cube/bundles/stlink-gdbserver/7.13.0+st.3/bin/ST-LINK_gdbserver` |
| J-Link gdbserver | `/usr/bin/JLinkGDBServer` |
| arm-none-eabi-gdb | `~/.local/share/stm32cube/bundles/gnu-tools-for-stm32/13.3.1+st.9/bin/arm-none-eabi-gdb` |
| Programmer | `~/.local/share/stm32cube/bundles/programmer/2.22.0+st.1/bin` |
| SVD | `~/.local/share/stm32cube/svd/STM32F429.svd` |

> Note: these are versioned bundle paths. If the STM32 extension updates its
> toolchain, update the version folders in `launch.json` to match (see step 2).
