import os

os.chdir("build")

if os.system("./simpleLang ../example_simpleLang_program.sl > temp.asm"):
    exit()

os.chdir("../deps/8bit-computer/8bit-computer")

os.system("asm/asm.py ../../../build/temp.asm > memory.list")

os.system("make clean && make run")