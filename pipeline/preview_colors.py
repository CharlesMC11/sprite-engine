"""
Quantize a list of hexadecimal color codes from 24‑bit colors to their 16‑bit
and 8‑bit packed representations. Then, display the original, quantized, and
expanded hexadecimal codes in a tabular format along with their ANSI color
representations.
"""

from argparse import ArgumentParser
from collections import namedtuple
from collections.abc import Iterable

from pipeline import (
    MAX_2BIT,
    MAX_4BIT,
    MAX_5BIT,
    MAX_6BIT,
    SHIFT_GREEN,
    SHIFT_RED,
    remap_from_8bit,
    remap_to_8bit,
)

# Types


Color = namedtuple("Color", "r g b")


# Public functions


def preview_colors(hex_codes: Iterable[str]) -> None:
    """Quantize each color and display it."""

    grid_top = (
        "┌"
        + "─" * 9
        + ("┬" + "─" * 16)
        + "┬"
        + ("─" * 6)
        + ("┬" + "─" * 14) * 3
        + "┬"
        + "─" * 10
        + "┐"
    )
    grid_sep = grid_top.replace("┌", "├").replace("┬", "┼").replace("┐", "┤")
    header = f"│ {'Entered':<7} │ {'RGB565':^14} │ {'Diff':^4} │ {'RGB332':^12} │ {'RGB422':^12} │ {'RGB224':^12} │ {'Pal 8':^8} │"

    print(grid_top, header.rstrip(), grid_sep, sep="\n")

    unique_codes: tuple[Color, ...] = filter_hex_codes(hex_codes)
    for color in unique_codes:
        print("│", hex_str(*color), end="")
        print_16bit(color)
        print_8bit(color)

    palette_size = len(unique_codes)
    bit_count = max(palette_size - 1, 0).bit_length()
    unit = "bit" if bit_count == 1 else "bits"

    print(
        grid_sep.replace("┼", "┴"),
        f"│ {f'{palette_size} colors ({bit_count} {unit})':<{len(grid_top) - 4}} │",
        f"└{'─' * (len(grid_top) - 2)}┘",
        sep="\n",
    )


def filter_hex_codes(raw_hex_codes: Iterable[str]) -> tuple[Color, ...]:
    result: set[Color] = set()

    for raw_code in raw_hex_codes:
        cleaned_code = raw_code.strip().removeprefix("#")

        if len(cleaned_code) != 6:
            continue
        try:
            r, g, b = bytes.fromhex(cleaned_code)
        except ValueError:
            continue

        result.add(Color(r, g, b))

    return tuple(sorted(result))


def hex_str(r: int, g: int, b: int) -> str:
    return f"#{r:02X}{g:02X}{b:02X}"


def print_16bit(color: Color) -> None:
    """Print the 16‑bit packed color

    :param color: The original color value
    """

    rq = remap_from_8bit(color.r, dst_max=MAX_5BIT)
    gq = remap_from_8bit(color.g, dst_max=MAX_6BIT)
    bq = remap_from_8bit(color.b, dst_max=MAX_5BIT)
    quantized = Color(rq, gq, bq)

    re = remap_to_8bit(rq, src_max=MAX_5BIT)
    ge = remap_to_8bit(gq, src_max=MAX_6BIT)
    be = remap_to_8bit(bq, src_max=MAX_5BIT)
    expanded = Color(re, ge, be)

    _print_codes_16bit(color, quantized, expanded)


def print_8bit(color: Color) -> None:
    """Print the 8‑bit packed color

    :param color: The original color value
    """

    max_3bit = MAX_4BIT >> 1

    q4 = _quantize(color, MAX_4BIT)
    e4 = _dequantize(q4, MAX_4BIT)

    q3 = _quantize(color, max_3bit)
    e3 = _dequantize(q3, max_3bit)

    q2 = _quantize(color, MAX_2BIT)
    e2 = _dequantize(q2, MAX_2BIT)

    _print_codes_8bit(
        color,
        q4,
        q3,
        q2,
        e4,
        e3,
        e2,
    )


def _quantize(color: Color, dst_max: int) -> Color:
    return Color(*(remap_from_8bit(c, dst_max) for c in color))


def _dequantize(color: Color, src_max: int) -> Color:
    return Color(*(remap_to_8bit(c, src_max) for c in color))


def _print_codes_16bit(
    original: Color, quantized: Color, expanded: Color
) -> None:
    """Print the 16‑bit packed color

    :param original: The original color value
    :param quantized: The quantized color value
    :param expanded: The expanded color value
    """

    hex_code = hex_str(expanded.r, expanded.g, expanded.b)
    packed = (
        (quantized.r << SHIFT_RED) | (quantized.g << SHIFT_GREEN) | quantized.b
    )
    patches = f"{_ansi_str(*original)}{_ansi_str(*expanded)}"

    print(f" │ {hex_code} [{packed:04X}] │ {patches}", end="")


def _print_codes_8bit(
    original: Color,
    q4: Color,
    q3: Color,
    q2: Color,
    e4: Color,
    e3: Color,
    e2: Color,
) -> None:
    """Print the 8‑bit packed colors

    :param original: The original color value
    :param q4: The 4‑bit quantized color value
    :param q3: The 3‑bit quantized color value
    :param q2: The 2‑bit quantized color value
    :param e4: The 4‑bit expanded color value
    :param e3: The 3‑bit expanded color value
    :param e2: The 2‑bit expanded color value
    """

    rq4, gq4, bq4 = q4
    rq3, gq3, bq3 = q3
    rq2, gq2, bq2 = q2

    nq = (rq3 << 5) | (gq3 << 2) | bq2
    wq = (rq4 << 4) | (gq2 << 2) | bq2
    cq = (rq2 << 6) | (gq2 << 4) | bq4

    re4, ge4, be4 = e4
    re3, ge3, be3 = e3
    re2, ge2, be2 = e2

    ne = hex_str(re3, ge3, be2)
    we = hex_str(re4, ge2, be2)
    ce = hex_str(re2, ge2, be4)

    patches = (
        f"{_ansi_str(*original)}"
        f"{_ansi_str(re3, ge3, be2)}"
        f"{_ansi_str(re4, ge2, be2)}"
        f"{_ansi_str(re2, ge2, be4)}"
    )

    print(
        f" │ {ne} [{nq:02X}] │ {we} [{wq:02X}] │ {ce} [{cq:02X}] │ ",
        patches,
        " │",
        sep="",
    )


def _ansi_str(r: int, g: int, b: int) -> str:
    return f"\x1b[48;2;{r};{g};{b}m  \x1b[0m"


if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    parser.add_argument(
        "hex_codes",
        help="Hexadecimal color codes to quantize",
        nargs="+",
        type=str,
    )
    args = parser.parse_args()

    preview_colors(args.hex_codes)
