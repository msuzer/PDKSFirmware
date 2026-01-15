Import("env")
import os
import shutil

# Enable ccache in ESP-IDF builds
env["ENV"]["IDF_CCACHE_ENABLE"] = "1"

# Optional: set a project-local cache directory and limits
ccache_dir = os.path.join(env["PROJECT_DIR"], ".ccache")
env["ENV"]["CCACHE_DIR"] = ccache_dir
env["ENV"]["CCACHE_MAXSIZE"] = "2G"

# Ensure the directory exists (PlatformIO will not create it automatically)
try:
    os.makedirs(ccache_dir, exist_ok=True)
except Exception:
    pass

print("[extra] ccache enabled for ESP-IDF; dir:", ccache_dir)

# Try to ensure ccache is discoverable in PATH for the build process
def _ensure_ccache_on_path():
    # If already found, skip
    if shutil.which("ccache"):
        return

    candidate_paths = []
    userprofile = os.environ.get("USERPROFILE") or os.path.expanduser("~")
    if userprofile:
        candidate_paths.append(os.path.join(userprofile, "scoop", "shims"))
    # Common Chocolatey bin path
    candidate_paths.append(r"C:\\ProgramData\\chocolatey\\bin")
    # Fallback typical install location
    candidate_paths.append(r"C:\\Program Files\\ccache")

    current_path = env["ENV"].get("PATH", os.environ.get("PATH", ""))
    additions = []
    for p in candidate_paths:
        ccache_exe = os.path.join(p, "ccache.exe")
        if os.path.exists(ccache_exe) and p not in current_path:
            additions.append(p)

    if additions:
        env["ENV"]["PATH"] = os.pathsep.join(additions + [current_path])
        print("[extra] ccache path added:", os.pathsep.join(additions))

_ensure_ccache_on_path()
