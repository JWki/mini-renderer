-- Some global paths
ROOT_DIR            = path.getabsolute(path.join(os.getcwd()), "../")
BIN_DIR             = path.join(ROOT_DIR, "build/bin")
INTERMEDIATE_DIR    = path.join(ROOT_DIR, "build/intermediate")
WORKSPACE_DIR       = path.join(ROOT_DIR, "workspace")
SOURCE_DIR          = path.join(ROOT_DIR, "src")
THIRD_PARTY_DIR     = path.join(ROOT_DIR, "third_party")

-- @note these are the new kids
RUNTIME_DIR         = path.join(SOURCE_DIR, "Runtime")


-- Defaults for all projects
function project_defaults()
    location(WORKSPACE_DIR)
    debugdir(ROOT_DIR)
    configuration "Debug"
        targetdir(path.join(BIN_DIR, "debug"))
    configuration "Release"
        targetdir(path.join(BIN_DIR, "release"))
    configuration {}
    includedirs {
        path.join(THIRD_PARTY_DIR, "include"),
        SOURCE_DIR,
    }
    files {
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "remotery"), "**.h")),
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "remotery"), "**.cpp"))
        --path.join(THIRD_PARTY_DIR, path.join(path.join("include", "microprofile"), "**.h")),
        --path.join(THIRD_PARTY_DIR, path.join(path.join("include", "microprofile"), "**.cpp"))
    }
    if os.isdir(path.join(ROOT_DIR, "livepp")) then
        defines "GT_LIVEPP"
    end
    defines { "UNICODE", "_HAS_EXCEPTIONS=0" } 
    flags {
        "FatalWarnings",
        "ExtraWarnings",
        "NoExceptions",
        --"NoImportLib",
        "NoPCH",
        "NoRTTI",
        --"PedanticWarnings",
        "Unicode",
        "UseFullPaths"
    }
    configuration "Debug"
        flags { "Symbols" }
        defines {"GT_DEBUG"}
    configuration "Release"
        flags { "OptimizeSpeed", "No64BitChecks" }
        defines { "NDEBUG" }
    configuration {}
    linkoptions {
        "/ignore:4199", -- LNK4199: no imports found from *.dll
        "/ignore:4668", -- C4668: symbol not defined as a preprocessor macro
        "/ignore:4100", -- C4100: unreferenced formal parameter
        -- livepp 
        "/FUNCTIONPADMIN",
        "/OPT:NOREF",
        "/OPT:NOICF",
        "/DEBUG:FULL"
    }
    buildoptions_cpp {
        "/std:c++17",   -- 
        "/wd4100",      -- C4100: unreferenced formal parameter
        "/wd4201",      -- C4202: nonstandard extension used: nameless struct/union
        "/wd4324",      -- C4324: structure was padded due to alignment specifier
        "/wd4091",      -- C4091: __declspec(dllexport) ignored on left of ... when no variable is declared
        "/wd4706",      -- C4706: assignment within conditional expression
        "/wd4251",      -- C4251: struct A needs to have dll-interface to be used by clients of struct B
    }
    configuration "Release"
    buildoptions_cpp {
        "/wd4189",       -- C4189: local variable is initialized but not referenced (imgui.cpp)
    }
    configuration {}
end -- project_defaults

-- PhysX linkage
function link_physX()
    configuration "Debug"
        links {
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/LowLevel_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/LowLevelAABB_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/LowLevelDynamics_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysX_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXCharacterKinematic_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXCommon_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXCooking_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXExtensions_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXFoundation_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXPvdSDK_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXTask_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/PhysXVehicle_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/SceneQuery_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_lib/SimulationController_static_64"),
        }
        postbuildcommands {
            "xcopy /y /d /f /i \"" .. path.join(THIRD_PARTY_DIR, "lib/PhysX/debug_dll/*.dll") .. "\" \"" .. path.join(BIN_DIR, "debug") .. "\"" 
        }
    configuration "Release"
        links {
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/LowLevel_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/LowLevelAABB_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/LowLevelDynamics_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysX_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXCharacterKinematic_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXCommon_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXCooking_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXExtensions_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXFoundation_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXPvdSDK_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXTask_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/PhysXVehicle_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/SceneQuery_static_64"),
            path.join(THIRD_PARTY_DIR, "lib/PhysX/release_lib/SimulationController_static_64"),
        }
        postbuildcommands {
            "xcopy /y /d /f /i \"" .. path.join(THIRD_PARTY_DIR, "lib/PhysX/release_dll/*.dll") .. "\" \"" .. path.join(BIN_DIR, "release") .. "\"" 
        }
    configuration {}
end -- link_physX

function add_imgui()
    files {
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "imgui"), "**.c")),
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "imgui"), "**.cpp")),
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "imgui"), "**.h"))
    }
end -- add_imgui

function add_csjon()
    files {
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "cjson"), "**.c")),
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "cjson"), "**.h"))
    }
end -- add_cjson

function add_eastl()
    files {
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "EASTLSource"), "**.cpp")),
        path.join(THIRD_PARTY_DIR, path.join(path.join("include", "EASTLSource"), "**.h"))
    }
end -- add_eastl

-- Main solution
solution "mini"
    location(WORKSPACE_DIR)
    configurations {
        "Debug",
        "Release"
    }
    platforms {
        "x64"
    }
    language "C++"
    startproject "Runtime"

    -- ---------------------
    group "Runtime"
        -- ---------------------
        -- Main executable
        project "mini"
            kind "WindowedApp"
            flags {
                "WinMain"
            }
            project_defaults()
            --links {
            --    "core_lib", "Core", "Graphics", "Physics"
            --}
            add_imgui()
            add_eastl()
            --add_csjon()
            files {
                path.join(RUNTIME_DIR, "**.c"),
                path.join(RUNTIME_DIR, "**.cpp"),
                path.join(RUNTIME_DIR, "**.h"),
            }
     
