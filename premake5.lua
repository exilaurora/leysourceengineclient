solution "leysourceengineclient"
   language "C++"
   location "project"
   targetdir "build/release"

   minimalrebuild "Off"
   buffersecuritycheck "Off"

   symbols "On"
   optimize "On"
   omitframepointer "On"
   vectorextensions "SSE2"
   floatingpoint "Fast"

   if os.istarget("linux") or os.istarget("macosx") then
      buildoptions {"-m32 -fPIC -ldl -lstdc++"}
      linkoptions  {"-m32 -fPIC -ldl -lstdc++"}
   end

   -- Use pkg-config to obtain lua flags and libs
   local lua_cflags = os.outputof("pkg-config --cflags lua"):gsub("\n"," ")
   local lua_libs   = os.outputof("pkg-config --libs lua"):gsub("\n"," ")

   if lua_cflags ~= "" then
      buildoptions { lua_cflags }
   end
   if lua_libs ~= "" then
      linkoptions { lua_libs }
   end

   vpaths {
      ["Header Files/*"] = "src/**.h",
      ["Source Files/*"] = "src/**.cpp",
   }

   kind "ConsoleApp"

   configurations { "Debug", "Release" }

   files { "src/**.h", "src/**.cpp" }

   libdirs {"libs/"}

   if os.istarget("windows") then
      links { "Shlwapi", "ws2_32", "winmm" }
   end

   -- A project defines one build target
   project "leysourceengineclient"
      targetname "leysourceengineclient"

      filter "configurations:Release"
         defines { "_CRT_SECURE_NO_WARNINGS", "NDEBUG", "_GENERIC" }
         symbols "Off"
         targetdir "build/release"

      filter "configurations:Debug"
         defines { "_CRT_SECURE_NO_WARNINGS", "DEBUG", "_GENERIC" }
         optimize "Off"
         targetdir "build/debug"

      filter {}