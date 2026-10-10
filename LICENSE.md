# MIT License

Copyright (c) 2026 Wonder Diaz (MetalSyntax)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Third-party components

The MIT License above covers this project's own source code. The following
components are distributed with it under their own licenses:

| Component | Location | License |
|---|---|---|
| [vitaGL](https://github.com/Rinnegatamante/vitaGL) by Rinnegatamante | `lib/vitagl/` | GNU LGPL v3.0 (`lib/vitagl/COPYING.LESSER`) |
| [vitaShaRK](https://github.com/Rinnegatamante/vitaShaRK) by Rinnegatamante | `lib/vitashark/` | GNU LGPL v3.0 (`lib/vitashark/LICENSE`) |
| [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P) font by CodeMan38 | `assets/PressStart2P-Regular.ttf`, `include/FontData.hpp` (bitmap derived from it) | SIL Open Font License 1.1 |

vitaGL and vitashark are statically linked into the VPK. They are included as
git submodules pinned to upstream commits (vitaGL `dca4b9d`, vitaShaRK
`df24065`); the only local change is `patches/vitagl-disable-system-app-mode.patch`.
Together with this project's full source, this lets anyone rebuild the game
against modified versions of those libraries, as the LGPL requires.
