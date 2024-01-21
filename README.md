start date: 12/17/23
<p> i got inspiration for this project from tsoding: https://youtube.com/tsodingdaily </p>
<p>the name of this build tool is NoMake™. 
This build tool is simple and can be used to compile multiple directories, you can even call different compilers that look for a different extension in the same directory!
</p>

<p> to build this project you will need:</p>
<ol><li>a c compiler, a build recipe, there is an example one in /build.c </li> <li> the top of nomake.h will show all the functions that are available, the lower part shows the implementation of these functi
o
ns</li> <li>to use this project you will need a build recipe and to put `#define COMPILATION_IMPLEMENTATION` before you include the header nomake.h, this will always be in nomake.h and will also be ./build.c 
and at the top of nomake.h</li> </ol>

<p> this is a small build recipe, the recipe will rebuild itself using GO_REBUILD, then the first compile_dir command does this: it looks for files in the current directory or ".", then it will put the binary
in "bin", it will use tcc as a compiler then and it will check for files with a ".c" extension to compile files</p>

![](img/build_recipe.png)


