$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$sourceIcon = Join-Path $repoRoot "assets\\icon.png"
$targetIcon = Join-Path $repoRoot "assets\\icon.ico"

@"
from PIL import Image

src = Image.open(r"$sourceIcon").convert("RGBA")
size = max(src.size)
canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
offset = ((size - src.width) // 2, (size - src.height) // 2)
canvas.paste(src, offset, src)
canvas.save(
    r"$targetIcon",
    sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)],
)
print(f"generated {r'$targetIcon'}")
"@ | uv run --with pillow==11.3.0 python -
