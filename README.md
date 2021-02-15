# bootsplash-manager
Simple bootsplash manager for Manjaro
<p align="center">
   <img src="https://raw.githubusercontent.com/ANDRoid7890/bootsplash-manager/master/images/bootsplash-manager-gui.svg"/>
</p>

## Features
### gui
<p align="center">
   <img src="https://github.com/ANDRoid7890/bootsplash-manager/blob/master/screenshots/main%20window.png?raw=true"/>
</p>

### cli
* Check if bootsplash is enabled and supported by current kernel:  `bootsplash-manager --status`
* List installed themes                                            `bootsplash-manager --list`
* Enable bootsplash and change current theme                       `bootsplash-manager --set <theme_name>`
* Disable bootsplash ( black screen )                              `bootsplash-manager --disable`
* Disable bootsplash ( log )                                       `bootsplash-manager --set-log`
  


## Installation
Make sure you have installed `bootsplash-systemd`. It's available in the official extra repo for Manjaro, for Arch - in AUR.

Make sure your current kernel supports bootsplash:
```bash
zgrep CONFIG_BOOTSPLASH /proc/config.gz
```
If you see `CONFIG_BOOTSPLASH=y` in the output - it does.

If it's not:
- on Manjaro use official one `pacman -S linux-latest`
- on Arch you can build one of custom kernels from AUR: `linux-bootsplash` 	`linux-manjaro-xanmod`
### Install
AUR package `bootsplash-manager-bin`.

## Usage
1) Search for some themes: `pacman -Ss bootsplash-theme-`
2) Install the ones you like `pacman -S bootsplash-theme-<theme_name> bootsplash-theme-<theme_name2> ...`
3) Set theme `bootsplash-manager -s <theme_name>`

## Building
```bash
git clone https://github.com/ANDRoid7890/bootsplash-manager.git
cd bootsplash-manager
cmake .
make
```
## TODO
- [x] gui
- [ ] install/remove dialog
- [ ] theme preview
- [ ] translations
