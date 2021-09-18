from conans import ConanFile


class ProjConan(ConanFile):
    requires = (
        "gcem/1.13.1",
        "miniz/2.1.0",
    )

    build_requires = (
        "gcem/1.13.1",
        "miniz/2.1.0",
    )

    generators = "cmake"

    def configure(self):
        pass
