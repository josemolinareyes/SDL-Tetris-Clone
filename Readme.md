# 🕹️ Cross-Platform Tetris Clone
**SDL2 Port**  

---

## 🚀 Features  
- 🎮 Authentic rotation/collision mechanics  
- 🌈 Color-coded Tetris pieces  
- 📈 Progressive difficulty (speed + level)  
- 👁️🗨️ Next piece preview panel  
- 💥 Particle effects on line clears  
- 🌐 Multi-platform support  

---

## 🖥️ Supported Platforms  

| Platform       | Status          | Requirements                          |  
|----------------|-----------------|---------------------------------------|  
| **Windows**    | ✅ Stable       | [SDL2 Runtime](https://libsdl.org/)   |  
| **Linux**      | ✅ Stable       | `libsdl2-dev`, `libsdl2-ttf-dev`      |  
| **macOS**      | ✅ Stable       | SDL2 via Homebrew                     |  
| **Web**        | 🚧 Experimental| [Emscripten](https://emscripten.org)   |  
| **Mobile**     | 🚧 In Progress  | Android NDK/Xcode                     |  

---

## 🔨 Build Instructions  

### Windows  
```bash  
gcc -o tetris.exe tetris.c -lSDL2 -lSDL2_ttf  
```

### Linux/macOS  
```bash  
# Debian/Ubuntu  
sudo apt install libsdl2-dev libsdl2-ttf-dev  

# macOS  
brew install sdl2 sdl2_ttf  

# Build  
gcc -o tetris tetris.c -lSDL2 -lSDL2_ttf  
```

---

## 🎮 Controls  

| Key            | Action          |  
|----------------|-----------------|  
| ← →           | Move piece      |  
| ↑             | Rotate          |  
| ↓             | Soft drop       |  
| Space         | Hard drop       |  
| Esc           | Quit game       |  

---

## 🌍 Portability  

### Mobile Requirements  
```c  
// Example touch handling  
SDL_TouchFingerEvent finger = event.tfinger;  
if(finger.type == SDL_FINGERDOWN) {  
    handle_touch(finger.x * SCREEN_W, finger.y * SCREEN_H);  
}  
```

### Web Build  
```bash  
emcc tetris.c -s USE_SDL=2 -s USE_SDL_TTF=2 -o tetris.html  
```

---

## ⚠️ Limitations  
- 📱 Mobile: Requires touch area optimization  
- 🖥️ HiDPI: Needs manual scaling configuration  
- 🔊 Audio: Not implemented (SDL_mixer needed)  

---

## ⚠️ Requirements
* arial.ttf font file in executable directory
* SDL2 runtime libraries

---

## 📜 Credits  
- Original code (C) 1995, Vadim Antonov
- Port to RetroBSD (C) 2015, Serge Vakulenko
- Port to Windows (command line version) and SDL (C) 2025, José Molina Reyes
- Development assistance: DeepSeek-R1 AI (https://www.deepseek.com)

If you have bug reports or want to contribute to this project, send me an email: josemolinareyes@riseup.net

Tetris is a registered trademark of The Tetris Company, in no way this work is related to the owners of the intellectual property or wants to infringe on their copyright.
