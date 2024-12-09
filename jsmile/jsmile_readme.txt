To build jSMILE on Linux:
1) copy the public SMILE files to cppsmile directory
2) set JINC variable to the JDK/include location
3) run build script
4) optionally verify the results with verify script
5) set LD_LIBRARY_PATH variable to ensure the .so is found.


To build jSMILE on Win32:
1) configure the C++ include directories in Visual Studio: add JDK's include and include/win32 to the list
2) unpack SMILE distribution for VC 7.1 in jsmile/cppsmile
3) use the jsmile_public.sln, which compiles both java and C++ sources
