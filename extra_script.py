Import("env")
import os

# arduino-esp32 3.x added a "Network" library that WiFi.h depends on.
# PlatformIO does not automatically include it when the WiFi framework library
# pulls it in.

arduino_libs = os.path.join(
    env.subst("$PROJECT_PACKAGES_DIR"),
    "framework-arduinoespressif32",
    "libraries"
)
if not os.path.isdir(arduino_libs):
    Return()

# 1. Add ALL bundled-library src/ dirs globally so cross-library #includes resolve.
for lib_name in sorted(os.listdir(arduino_libs)):
    lib_src = os.path.join(arduino_libs, lib_name, "src")
    if os.path.isdir(lib_src):
        env.Append(CPPPATH=[lib_src])

# 2. Only compile and link Network for environments that use WiFi (web_node).
if env.subst("$PIOENV") != "web_node":
    Return()

network_src = os.path.join(arduino_libs, "Network", "src")
if not os.path.isdir(network_src):
    Return()

build_dir = env.subst("$BUILD_DIR")
obj_dir   = os.path.join(build_dir, "NetworkExtra")

network_objects = []
for fname in sorted(os.listdir(network_src)):
    if not fname.endswith(".cpp"):
        continue
    src = os.path.join(network_src, fname)
    obj = os.path.join(obj_dir, fname.replace(".cpp", ".o"))
    network_objects.append(env.Object(target=obj, source=src))

network_lib = env.Library(
    target=os.path.join(build_dir, "libNetworkExtra"),
    source=network_objects,
)
env.Prepend(LIBS=[network_lib])
