"""
Asset Interpreter.

Preview a specialized binary format.

The binary layout contains:
- Header (24 bytes):
    - Magic (8 bytes)
    - Sprite metadata (16 bytes)
        - Bounding box (4 bytes)
            - u_min, u_max (2 bytes)
            - v_min, v_max (2 bytes)
        - Anchor (8 bytes)
            - u_anchor (4 bytes)
            - v_anchor (4 bytes)
        - Depth (1 byte)
        - Physics type (1 byte)
        - Color encoding (1 byte)
        - Palette index (1 byte), set to 0x3F (ASCII for '?') as a placeholder
- Pixels (height×width bytes): 1‑byte packed values [*][E][AA][IIII]
- Footer (34 bytes): additional metadata
    - Color palette (32 bytes): 16 2‑byte unique colors
    - Width (1 byte): width in pixels
    - Height (1 byte): height in pixels
"""

import math
from argparse import ArgumentParser
from pathlib import Path
from warnings import warn

import cv2
import numpy as np
import numpy.typing as npt

from pipeline import (
    FOOTER_SIZE_BYTES,
    FRACTIONAL_BITS,
    MASK_ALPHA,
    MASK_PALETTE_INDEX,
    MAX_2BIT,
    MAX_8BIT,
    SHIFT_ALPHA,
    SPRITE_DIMENSIONS_SIZE_BYTES,
    BGRImage,
    ColorEncoding,
    ResourceLayoutWarning,
    SpriteMetadata,
    get_encoding_params,
    is_power_of_2,
    remap_to_8bit,
)

TARGET_HEIGHT: int = 512


def decompile_asset(source_path: Path) -> tuple[BGRImage, SpriteMetadata]:
    """
    Decompile the asset into a viewable image.

    :param source_path: The path to the asset to decompile.

    :returns: The viewable image.
    """

    buffer = source_path.read_bytes()
    meta = SpriteMetadata.from_bytes(buffer)

    pixels_blob = buffer[
        SpriteMetadata.EXPECTED_SIZE_BYTES : -FOOTER_SIZE_BYTES
    ]
    palette_blob = buffer[-FOOTER_SIZE_BYTES:-SPRITE_DIMENSIONS_SIZE_BYTES]
    width, height = buffer[-SPRITE_DIMENSIONS_SIZE_BYTES:]

    if not (is_power_of_2(width) and is_power_of_2(height)):
        warn(
            f"'{source_path.name}' has invalid dimensions: {width}×{height}!",
            ResourceLayoutWarning,
        )

    pixels = np.frombuffer(pixels_blob, dtype=np.uint8)
    index_bits = pixels & MASK_PALETTE_INDEX

    alpha_bits = ((pixels & MASK_ALPHA) >> SHIFT_ALPHA).astype(np.uint16)
    alpha_mask = remap_to_8bit(alpha_bits, MAX_2BIT).reshape((height, width))

    palette = _unpack_16bit_to_color(palette_blob, meta.color_encoding)
    image_bgr = palette[index_bits].reshape((height, width, 3))

    return cv2.merge((image_bgr, alpha_mask)).astype(np.uint8), meta


# Protected helpers


def _unpack_16bit_to_color(
    packed_buffer: bytes, encoding: ColorEncoding
) -> BGRImage:
    """
    Unpack 2‑byte integers into 1‑byte BGR channels.

    :param packed_buffer: The buffer containing the 2‑byte integers.
    :param encoding: The color encoding to use.

    :returns: The unpacked BGR image.
    """

    r_max, g_max, b_max, r_shift, g_shift = get_encoding_params(encoding)

    b: npt.NDArray[np.uint16]
    g: npt.NDArray[np.uint16]
    r: npt.NDArray[np.uint16]

    packed_color = np.frombuffer(packed_buffer, dtype=np.uint16)
    r = remap_to_8bit((packed_color >> r_shift) & r_max, r_max)
    g = remap_to_8bit((packed_color >> g_shift) & g_max, g_max)
    b = remap_to_8bit(packed_color & b_max, b_max)

    return np.stack((b, g, r), axis=-1)


if __name__ == "__main__":
    parser = ArgumentParser("Asset Interpreter", description=__doc__)
    parser.add_argument(
        "source_sprite", type=Path, help="Path to the source sprite file."
    )

    args = parser.parse_args()

    if not (path := args.source_sprite).exists():
        raise FileNotFoundError(f"Missing asset file: '{path}'")

    image, meta = decompile_asset(path)

    # TODO: Hide scaling in a function later
    scale_factor = TARGET_HEIGHT / image.shape[0]
    target_width = round(image.shape[1] * scale_factor)

    image_resized = cv2.resize(
        image,
        (target_width, TARGET_HEIGHT),
        interpolation=cv2.INTER_NEAREST,
    )

    # TODO: Hide annotation in a function later
    cv2.rectangle(
        image_resized,
        (
            math.floor(meta.bbox.u_min * scale_factor),
            math.floor(meta.bbox.v_min * scale_factor),
        ),
        (
            math.ceil((meta.bbox.u_max + 1) * scale_factor),
            math.ceil((meta.bbox.v_max + 1) * scale_factor),
        ),
        color=(0x00, 0x00, MAX_8BIT),
        thickness=1,
    )

    u_anchor = meta.u_anchor / (1 << FRACTIONAL_BITS)
    v_anchor = meta.v_anchor / (1 << FRACTIONAL_BITS)
    cv2.circle(
        image_resized,
        (
            math.floor(u_anchor * scale_factor),
            math.floor(v_anchor * scale_factor),
        ),
        radius=0,
        color=(0x00, MAX_8BIT, 0x00),
        thickness=8,
    )

    cv2.imshow(f"Preview: {path.name}", image_resized)
    cv2.waitKey(0)
