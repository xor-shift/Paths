from conans import ConanFile

class ProjConan(ConanFile):
    settings = {
        "os": ["Linux", "Windows"],
        "arch": ["x86", "x86_64"],
    }

    requires = (
        "luajit/2.0.5",
        #"lua/5.4.3",
        "miniz/2.1.0",
        "sol2/3.2.3",
    )

    build_requires = (
        "luajit/2.0.5",
        #"lua/5.4.3",
        "miniz/2.1.0",
        "sol2/3.2.3",
    )

    generators = "cmake"

    def configure(self):
        self.options["lua"].shared = True
        self.options["luajit"].shared = True
        pass
