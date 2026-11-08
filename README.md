# Ground Up C++

An interactive textbook that takes you from **no C++ at all** (a little Python is enough) to **systems-level C++**: memory and ownership, modern C++, performance engineering, and real projects such as a game-engine core, a software renderer, a machine-learning library with autograd, fast matrix multiplication, GPU kernels and a low-latency order-matching engine.

**Read it:** open `index.html`, or the GitHub Pages site for this repository.

- Every example is compiled with g++ 13 (`-std=c++23 -O2 -Wall -Wextra`) and run when the book is built; the output shown is real.
- Every runnable example has an *Open in Compiler Explorer* link, so you can edit and run it in your browser.
- Interactive parts: memory steppers, a bit viewer, predict-the-output exercises, quizzes, and more.
- `examples/` holds every example as a `.cpp` file you can compile yourself.

## Building the book

```bash
pip install markdown pygments
python3 tools/build.py          # compiles and runs every example, writes chapters/*.html and index.html
python3 tools/build.py 08 09    # rebuild only chapters whose file name starts with 08 or 09
```

Chapter sources are Markdown files in `src/`; the course outline is `tools/toc.json`.
