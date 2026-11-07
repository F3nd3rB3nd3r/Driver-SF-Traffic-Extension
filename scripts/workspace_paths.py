"""Locate preserved historical evidence after the 2026-11-06 workspace archive."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE_ROOT = ROOT / 'archive/retired_2026-11-06'


def historical_path(relative):
    """Prefer an original path; otherwise use its archived, unchanged copy."""
    relative = Path(relative)
    if relative.is_absolute() or relative.drive or '..' in relative.parts:
        raise ValueError('Expected a workspace-relative evidence path')
    for candidate in (ROOT / relative, ARCHIVE_ROOT / relative):
        if candidate.is_file():
            return candidate
    raise FileNotFoundError(f'Historical evidence not found: {relative}')
