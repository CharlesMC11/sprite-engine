"""
Quantize a list of hexadecimal color codes from 24‑bit colors to their 16‑bit
and 8‑bit packed representations. Then, display the original, quantized, and
expanded hexadecimal codes in a tabular format along with their ANSI color
representations.
"""

from argparse import ArgumentParser
from collections import namedtuple
from collections.abc import Iterable

from pipeline import remap

# Types


Color = namedtuple("Color", "r g b")


# Public functions


def main() -> None:
    parser = ArgumentParser(description=__doc__)
    parser.add_argument(
        "hex_codes",
        help="Hexadecimal color codes to quantize",
        nargs="+",
        type=str,
    )
    args = parser.parse_args()

    preview_colors(args.hex_codes)


def preview_colors(hex_codes: Iterable[str]) -> None:
    """Quantize each color and display it."""

    grid_top = (
        "┌"
        + "─" * 9
        + ("┬" + "─" * 16) * 3
        + "┬"
        + ("─" * 10)
        + ("┬" + "─" * 14) * 3
        + "┬"
        + "─" * 10
        + "┐"
    )
    grid_sep = grid_top.replace("┌", "├").replace("┬", "┼").replace("┐", "┤")
    header = f"│ {'Entered':<7} │ {'Neutral 16':^14} │ {'Warm 16':^14} │ {'Cool 16':^14} │ {'Pal 16':^8} │ {'Neutral 8':^12} │ {'Warm 8':^12} │ {'Cool 8':^12} │ {'Pal 8':^8} │"

    print(
        grid_top,
        header.rstrip(),
        grid_sep,
        sep="\n",
    )

    unique_codes: tuple[Color] = filter_hex_codes(hex_codes)
    for color in unique_codes:
        print("│", hex_str(*color), end="")
        print_16bit(color)
        print_8bit(color)

    palette_size = len(unique_codes)
    bit_count = max(palette_size - 1, 0).bit_length()
    unit = "bit" if bit_count == 1 else "bits"

    print(
        grid_sep.replace("┼", "┴"),
        f"│ {f'{palette_size} colors ({bit_count} {unit})':<125} │",
        f"└{'─' * (len(grid_top) - 2)}┘",
        sep="\n",
    )


def filter_hex_codes(raw_hex_codes: Iterable[str]) -> tuple[Color]:
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
    r, g, b = color

    max6 = (1 << 6) - 1
    q6 = _quantize(r, g, b, dst_max=max6)
    e6 = _dequantize(*q6, src_max=max6)

    max5 = max6 >> 1
    q5 = _quantize(r, g, b, dst_max=max5)
    e5 = _dequantize(*q5, src_max=max5)

    _print_codes_16bit(color, q6, q5, e6, e5)


def print_8bit(color: Color) -> None:
    """Print the 8‑bit packed color

    :param color: The original color value
    """
    r, g, b = color

    max4 = (1 << 4) - 1
    q4 = _quantize(r, g, b, dst_max=max4)
    e4 = _dequantize(*q4, src_max=max4)

    max3 = max4 >> 1
    q3 = _quantize(r, g, b, dst_max=max3)
    e3 = _dequantize(*q3, src_max=max3)

    max2 = max3 >> 1
    q2 = _quantize(r, g, b, dst_max=max2)
    e2 = _dequantize(*q2, src_max=max2)

    _print_codes_8bit(
        color,
        q4,
        q3,
        q2,
        e4,
        e3,
        e2,
    )


def _quantize(r: int, g: int, b: int, dst_max: int) -> Color:
    return Color(
        remap(r, src_max=0xFF, dst_max=dst_max),
        remap(g, src_max=0xFF, dst_max=dst_max),
        remap(b, src_max=0xFF, dst_max=dst_max),
    )


def _dequantize(r: int, g: int, b: int, src_max: int) -> Color:
    return Color(
        remap(r, src_max=src_max, dst_max=0xFF),
        remap(g, src_max=src_max, dst_max=0xFF),
        remap(b, src_max=src_max, dst_max=0xFF),
    )


def _print_codes_16bit(
    original: Color,
    q6: Color,
    q5: Color,
    e6: Color,
    e5: Color,
) -> None:
    """Print the 16‑bit packed color

    :param original: The original blue value
    :param q6: The 6‑bit quantized color value
    :param q5: The 5‑bit quantized color value
    :param e6: The 6‑bit expanded color value
    :param e5: The 5‑bit expanded color value
    """

    rq6, gq6, bq6 = q6
    rq5, gq5, bq5 = q5

    nq = (rq5 << 11) | (gq6 << 5) | bq5
    wq = (rq6 << 10) | (gq5 << 5) | bq5
    cq = (rq5 << 11) | (gq5 << 6) | bq6

    re6, ge6, be6 = e6
    re5, ge5, be5 = e5

    ne = hex_str(re5, ge6, be5)
    we = hex_str(re6, ge5, be5)
    ce = hex_str(re5, ge5, be6)

    patches = (
        f"{_ansi_str(*original)}"
        f"{_ansi_str(re5, ge6, be5)}"
        f"{_ansi_str(re6, ge5, be5)}"
        f"{_ansi_str(re5, ge5, be6)}"
    )

    print(
        f" │ {ne} [{nq:04X}] │ {we} [{wq:04X}] │ {ce} [{cq:04X}] │ ",
        patches,
        end="",
        sep="",
    )


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
    main()
