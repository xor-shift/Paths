from conans import ConanFile


class ProjConan(ConanFile):
    settings = "compiler"

    requires = ("gcem/1.13.1",
                )

    generators = "cmake"

    def configure(self):
        pass
