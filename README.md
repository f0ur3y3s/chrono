# chrono

Chrono is a Windows clock application that stays above any other window.

Start it and dive into your work while keeping track of the time.

## NOTES

Defender may think that this is a virus due to the loading and usage of an undocumented API function, `SetWindowCompositionAttribute`. I have not found a work-around for this yet, however, you can see that the only usage of a runtime module load is in [`src/chrono_window.c`](src/chrono_window.c).

## 🧰 Building

```bash
cmake -S. -Bbuild
cmake --build build --config Release
```

## ❌ Quitting

To quit, left click then hold, press `esc`.

## ⚙️ Configuration

In `main.h`, you can find defined variables containing user-changeable variables. Adjust these to your liking, compile, build, repeat.