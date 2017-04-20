project("utest")
    language("C++")
    uuid(os.uuid("any-vm-utest"))
    kind("ConsoleApp")
    removeflags { "NoExceptions" }
    configuration "gcc*"
        buildoptions { "-Wno-parentheses" }
    configuration {}
    includedirs {
        DIR.UTEST,
        DIR.INCLUDE
    }
    files {
        path.join(DIR.UTEST, "**.cpp"),
        path.join(DIR.UTEST, "**.c"),
        path.join(DIR.UTEST, "**.h")
    }
    links {
        "any-vm"
    }