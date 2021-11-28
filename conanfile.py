from conans import ConanFile

class ProjConan(ConanFile):
    settings = {
        "os": ["Linux", "Windows"],
        "arch": ["x86", "x86_64"],
    }

    requires = (
        "miniz/2.1.0",
        "sol2/3.2.3",
        "lua/5.4.3",
    )

    build_requires = (
        "miniz/2.1.0",
        "sol2/3.2.3",
        "lua/5.4.3",
    )

    generators = "cmake"

    def configure(self):
        pass
