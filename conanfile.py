from conans import ConanFile


class ProjConan(ConanFile):
    settings = {
        "os": ["Linux", "Windows"],
        "arch": ["x86", "x86_64"],
    }

    requires = (
        "gcem/1.13.1",
        "nlohmann_json/3.10.2",
        "miniz/2.1.0",
    )

    build_requires = (
        "gcem/1.13.1",
        "nlohmann_json/3.10.2",
        "miniz/2.1.0",
    )

    generators = "cmake"

    def configure(self):
        pass